/*--------------------------------------------------------------------------+
 | FILE NAME - amtr_mgr.c                                                   |
 |             address monitor manager                                      |
 +--------------------------------------------------------------------------+
 |                             2004/08/06 water_huang                       |
 +--------------------------------------------------------------------------*/
/*
 * History:
 *       Date        Modifier   Reason
 *  2004/08/06    water huang AMTR refinement, HW-learning -> SW-learning
 *
 *                <architecture>        --------------
 *                     AMTR_MGR  <=>   |  Hisam Table | [core layer]
 *                                      --------------
 *                                 =====================================
 *                                       -------------
 *                     AMTRDRV   <=>   |  Hash Table  | [driver layer]
 *                                       -------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "sys_adpt.h"
#include "sys_bld.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "sys_type.h"
#include "sysfun.h"
#include "l_hisam.h"
#include "leaf_2863.h"
#include "leaf_2674p.h"
#include "leaf_2674q.h"
#include "sysrsc_mgr.h"
#include "stktplg_pom.h"
#include "amtr_mgr.h"
#include "amtr_om.h"
#include "amtr_om_private.h"
#include "amtr_type.h"
#include "amtrdrv_mgr.h"
#include "amtrdrv_pom.h"
#include "swctrl.h"
#include "swctrl_pmgr.h"
#include "vlan_lib.h"
#include "vlan_om.h"
#include "xstp_om.h"
#include "sys_module.h"
#include "dev_nicdrv.h"
#include "backdoor_mgr.h"
#include "l2_l4_proc_comm.h"
#include "netaccess_group.h"
#include "sys_callback_mgr.h"
#include "swdrvl3.h"

#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
#include "trap_event.h"
#include "trap_mgr.h"
#endif

#if (SYS_CPNT_AMTR_VLAN_MAC_LEARNING == TRUE)
#include "swdrv.h"
#endif
#if (SYS_CPNT_VXLAN == TRUE)
#include "vxlan_type.h"
#include "vxlan_om.h"
#include "ipal_types.h"
#include "ipal_neigh.h"
#endif

#include "snmp_pmgr.h"

static UI8_T cpu_mac[SYS_ADPT_MAC_ADDR_LEN]={0};

#define AMTR_MGR_IS_CPUMAC(na)  (!memcmp(cpu_mac,na,SYS_ADPT_MAC_ADDR_LEN))
/* NAMING CONSTANT DECLARARTIONS
 */
#define AMTR_MGR_BACKDOOR_OPEN

#if 1 // wakka porting
/* Filtering entry which will be aged out.
 * Now only AMTR have filtering entry information.
 * Other CSCs can't get it.
 * AMTR will filter the kind of entry in "get exact",
 * "get next", "NA callback", "age out callback" functions.
 * But other CSCs can delete it by "delete addr" and "del_by_group"
 * function.
 * AMTR should follow this rule until UI changed.
 * (until UI support to display filtering entries.)
 * water_huang 2006.5.10
 */
#define AMTR_MGR_FILTER_OUT_DROP_ENTRY_FOR_UI          TRUE
#endif

/* EPR: ES4626F-SW-FLF-38-01373
 * Problem Description:
 *     ECS4610-26T unknwon vlan packet will be processed by CPU
 * Root Cause:
 *     If receives a unknown vlan packet
 *     (a.k.a. packets that belong to a core layer unaware VLAN),
 *     AMTR will not learn the MAC address.
 *     That is, continuous packets will lead to high CPU usage.
 * Solution:
 *     AMTR learn/sync/get/set MAC never filter out addresses by VLAN
 *     existence or STA state.
 */
#define AMTR_MGR_VLAN_VALIDATION                        FALSE
#define AMTR_MGR_STP_VALIDATION                         TRUE

#if (SYS_CPNT_ADD == TRUE || SYS_CPNT_AMTRL3 == TRUE)
#define AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY    TRUE
#endif

/* MlagMacNotifyWA
 * This MlagMacNotify mechanism is limit by mac_ntfy_rec_lst size,
 * excess of changed entries can't be tracked.
 * Workaround is to send notification via MacAddrUpdateCallback.
 */
#if (SYS_CPNT_MLAG == TRUE)
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY != TRUE)
#define AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY    TRUE
#endif
#define AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY_FOR_MLAG    TRUE
#endif

/* event definition
 */
#define AMTR_MGR_EVENT_ENTER_TRANSITION_MODE     BIT_0    /* enter transition mode event*/
#if(AMTR_TYPE_MEMCPY_MEMCOM_PERFORMANCE_TEST == 1)
static UI32_T  amtrmgr_memcpy_counter = 0;
static UI32_T  amtrmgr_memset_counter = 0;
static UI32_T  amtrmgr_memcmp_counter = 0;
#define amtrmgr_memcpy(a,b,c)  do{ \
       amtrmgr_memcpy_counter ++;   \
       memcpy(a,b,c);   \
       }while(0)
#define amtrmgr_memset(a,b,c)  do{ \
           amtrmgr_memset_counter ++;   \
           memset(a,b,c);   \
           }while(0)
#define amtrmgr_memcmp(a,b,c)  (amtrmgr_memcmp_counter++, memcmp(a,b,c))
#else
#define amtrmgr_memcpy(a,b,c)  memcpy(a,b,c)
#define amtrmgr_memset(a,b,c)  memset(a,b,c)
#define amtrmgr_memcmp(a,b,c)  memcmp(a,b,c)
#endif
#if (AMTR_TYPE_DEBUG_PACKET_FLOW == 1)
static int amtr_mgr_na_interface_index = 0;
#define AMTR_MGR_IS_TRACED_INTERFACE(index) ((index == amtr_mgr_na_interface_index)||(amtr_mgr_na_interface_index == 55))
#endif


/* MACRO DEFINITIONS
 */
#define IS_MAC_INVALID(mac) ((mac[0]|mac[1]|mac[2]|mac[3]|mac[4]|mac[5])==0)  /*added by Jinhua.Wei,to remove warning*/
#if 0
#define IS_MAC_INVALID(mac)     (amtrmgr_memcmp((mac), null_mac, AMTR_TYPE_MAC_LEN) == 0)
#endif
#define IS_VID_INVALID(vid )    ((vid) == 0  || ((vid) > AMTR_TYPE_MAX_LOGICAL_VLAN_ID))
#define IS_MULTICAST_MAC(mac)   ((mac)[0] & 0x01)
#if(SYS_CPNT_VXLAN == TRUE)
#define IS_IFINDEX_INVALID(ifindex)    ((ifindex == 0)||(ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT_INCLUDE_VXLAN))
#define IS_IFINDEX_INVALID_EXCLUDE_0(ifindex)    (ifindex > (SYS_ADPT_TOTAL_NBR_OF_LPORT_INCLUDE_VXLAN))
#else
#define IS_IFINDEX_INVALID(ifindex)    ((ifindex == 0)||(ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT))
#define IS_IFINDEX_INVALID_EXCLUDE_0(ifindex)    (ifindex > (SYS_ADPT_TOTAL_NBR_OF_LPORT))
#endif

#define IS_SOURCE_INVALID(source)      ((source<AMTR_TYPE_ADDRESS_SOURCE_INTERNAL)||(source>AMTR_TYPE_ADDRESS_SOURCE_SECURITY)||(source == AMTR_TYPE_ADDRESS_SOURCE_INVALID))
#define IS_LIFETIME_INVALID(life_time) ((life_time<AMTR_TYPE_ADDRESS_LIFETIME_OTHER)||(life_time > AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)||(life_time==AMTR_TYPE_ADDRESS_LIFETIME_INVALID))
#define IS_ACTION_INVALID(action)      (action> AMTR_TYPE_ADDRESS_ACTION_FORWARDING_AND_TRAP_TO_CPU)
#define IS_PROTOCOL_INVALID(protocol)  (protocol >= AMTR_MGR_PROTOCOL_MAX)

#define IS_FORWARDING_ENTRY(action)     (((action) == AMTR_TYPE_ADDRESS_ACTION_FORWARDING)||  \
                                         ((action) == AMTR_TYPE_ADDRESS_ACTION_FORWARDING_AND_TRAP_TO_CPU))

/* In HW Learning, these APIs are invalid or useless
 */
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
/* AMTR support HW Learning iff standard alone.
 */
#define AMTRDRV_MGR_SetAddrList2RemoteUnit(num_of_entries,addr_buf)
#endif
#ifdef AMTR_MGR_DEBUG
static int amtr_mgr_debug_return = 0;
#define AMTR_MGR_RETURN(value)               \
        do{                                  \
         if(amtr_mgr_debug_return){          \
            BACKDOOR_MGR_Printf("the function %s.the line %i\n",__FUNCTION__,__LINE__);\
            }                                \
         return value;                       \
        }while(0)
#else
#define AMTR_MGR_RETURN(value) \
     do{                      \
       return value ;           \
     } while(0)
#endif

#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
#define AMTR_MGR_NOTIFY_EDIT_ADDR_ENTRY_FROM_BUFFER(count, is_age, do_again_p) \
    do {                                                           \
        UI32_T i;                                                  \
        for (i = 0; i < (count); i++)                              \
        {                                                          \
            AMTR_MGR_Notify_EditAddrEntry(edit_entry_buf[i].vid, edit_entry_buf[i].mac, edit_entry_buf[i].ifindex, (is_age)); \
            AMTR_MGR_NOTIFY_MLAG_ADDR_ENTRY(&(edit_entry_buf[i]), (is_age)); \
        }                                                          \
        *(do_again_p) = (count) == SYS_ADPT_AMTR_NBR_OF_ADDR_TO_SYNC_IN_ONE_SCAN; \
    } while (0)
#endif

#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY_FOR_MLAG == TRUE)
#define AMTR_MGR_NOTIFY_MLAG_ADDR_ENTRY(entry, is_age) \
    do { \
        if (AMTR_TYPE_ADDRESS_SOURCE_MLAG != (entry)->source) { \
            AMTR_MGR_Notify_EditAddrEntryForMlag((entry)->vid, (entry)->mac, (entry)->ifindex, (is_age)); \
        } \
    } while (0)
#else
#define AMTR_MGR_NOTIFY_MLAG_ADDR_ENTRY(entry, is_age) ((void)0)
#endif

#define isdigit(c) ((c) >= '0' && (c) <= '9')  /* remove warning,dan.xie*/

#if (AMTR_MGR_VLAN_VALIDATION == TRUE)
#define AMTR_MGR_CHECK_VLAN_VALIDITY(vid)               VLAN_OM_IsVlanExisted(vid)
#else
#define AMTR_MGR_CHECK_VLAN_VALIDITY(vid)               TRUE
#endif

#if (AMTR_MGR_STP_VALIDATION == TRUE)
#define AMTR_MGR_CHECK_STP_STATE_VALIDITY(vid, lport)   AMTR_MGR_CheckStpState(vid, lport)
#else
#define AMTR_MGR_CHECK_STP_STATE_VALIDITY(vid, lport)   TRUE
#endif


/* LOCAL TYPES
 */

/* LOCAL FUNCTIONS DECLARATIONS
 */
static void AMTR_MGR_LPortToOctet               (UI8_T  *octet, UI32_T index);
static UI32_T AMTR_MGR_OctetToLPort             (UI8_T  *octet, UI32_T comparing_length);
static void AMTR_MGR_UpdatePortSecurityStatus(UI32_T ifindex);
#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)
static BOOL_T AMTR_MGR_Notify_IntrusionMac      (UI32_T src_lport, UI16_T vid, UI8_T *src_mac, UI8_T *dst_mac, UI16_T ether_type);
#endif
static void AMTR_MGR_Notify_PortMove(UI32_T num_of_entry, AMTR_TYPE_PortMoveEntry_T *entry_p);
static void AMTR_MGR_Notify_SecurityPortMove    (UI32_T ifindex, UI32_T vid, UI8_T *mac, UI32_T original_ifindex);
static void AMTR_MGR_Notify_AutoLearnCount      (UI32_T ifindex, UI32_T portsec_status);
static void AMTR_MGR_Notify_MACTableDeleteByPort(UI32_T ifindex,UI32_T reason);
static void AMTR_MGR_Notify_MACTableDeleteByVid (UI32_T vid);
static void AMTR_MGR_Notify_MACTableDeleteByVIDnPort(UI32_T vid, UI32_T ifindex);
static void AMTR_MGR_Notify_MacTableDeleteByLifeTime(UI32_T life_time);
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
static void AMTR_MGR_Notify_AgingOut(UI32_T num_of_entries, AMTR_TYPE_AddrEntry_T addr_buf[]);
#endif
static BOOL_T AMTR_MGR_Notify_SetStaticMACCheck(UI32_T vid, UI8_T *mac, UI32_T ifindex);
static void AMTR_MGR_Notify_EditAddrEntry(UI32_T vid, UI8_T *mac, UI32_T ifindex, BOOL_T is_age);
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY_FOR_MLAG == TRUE)
static void AMTR_MGR_Notify_EditAddrEntryForMlag(UI32_T vid, UI8_T *mac, UI32_T ifindex, BOOL_T is_age);
#endif

#if (AMTR_MGR_STP_VALIDATION == TRUE)
static BOOL_T AMTR_MGR_CheckStpState(UI32_T vid, UI32_T lport);
#endif
static void AMTR_MGR_MacNotifyAddNewEntry(
    UI32_T  ifidx,
    UI16_T  vid,
    UI8_T   *src_mac_p,
    BOOL_T  is_add,
    UI8_T   skip_mac_notify);
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
static void AMTR_MGR_MacNotifySendTrap(
    UI32_T  ifidx,
    UI16_T  vid,
    UI8_T   *src_mac_p,
    BOOL_T  is_add);
#endif /* #if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE) */

static BOOL_T AMTR_MGR_AuthenticatePacket(
    UI8_T *src_mac,
    UI32_T vid,
    UI32_T lport,
    SYS_CALLBACK_MGR_AuthenticatePacket_Result_T auth_result
);

#ifdef AMTR_MGR_BACKDOOR_OPEN
static void AMTR_MGR_BD_DumpHisam(void);
static UI32_T AMTR_MGR_BD_DisplayCallback (void *record, void *cookies);
static void AMTR_MGR_BD_DumpPortInfo(void);
static void AMTR_MGR_BD_SetPortInfo(void);
static void AMTR_MGR_BD_SetAddrEntry(void);
static void AMTR_MGR_BD_DelAddrEntry(void);
static void AMTR_MGR_BD_DelAddrByPort(void);
static void AMTR_MGR_BD_DelAddrByVid(void);
static void AMTR_MGR_BD_DelAddrByLifeTime(void);
static void AMTR_MGR_BD_DelAddrBySource(void);
static void AMTR_MGR_BD_DelAddrByPortAndLifeTime(void);
static void AMTR_MGR_BD_DelAddrByPortAndSource(void);
static void AMTR_MGR_BD_DelAddrByVidAndLifeTime(void);
static void AMTR_MGR_BD_DelAddrByVidAndSource(void);
static void AMTR_MGR_BD_DelAddrByLifeTimenVidnPort(void);
static void AMTR_MGR_BD_DelAddrBySourcenVidnPort(void);
static void AMTR_MGR_BD_DelAllAddr(void);
static void AMTR_MGR_BD_GetAddr(void);
static void AMTR_MGR_BD_GetNextMVAddr(void);
static void AMTR_MGR_BD_SetAgingStatus(void);
static void AMTR_MGR_BD_SetAgingTime(void);
static void AMTR_MGR_BD_SetCpuMac(void);
static void AMTR_MGR_BD_DeleteCpuMac(void);
static void AMTR_MGR_BD_IsPortSecurityEnabled(void);
static void AMTR_MGR_BD_PerformanceTesting(void);
static void AMTR_MGR_BD_CombineEventTesting(void);
static void AMTR_MGR_BD_ChangeArlCapacity(void);
static void AMTR_MGR_BackDoor_Menu(void);
#endif/*AMTR_MGR_BACKDOOR_OPEN*/
static BOOL_T AMTR_MGR_SetVlanLearningDefaultStatus(void);
static BOOL_T AMTR_MGR_SetAddrEntryWithPriorityAndMacNotify(AMTR_TYPE_AddrEntry_T *addr_entry, UI8_T skip_mac_notify);
static BOOL_T AMTR_MGR_DeleteAddrWithMacNotify(UI32_T vid, UI8_T *mac, UI8_T skip_mac_notify);
static BOOL_T AMTR_MGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Internal(UI32_T ifindex, UI16_T start_vid, UI16_T end_vid, AMTR_TYPE_AddressLifeTime_T life_time, BOOL_T sync_op);

/* STATIC VARIABLE DECLARATIONS
 */
const static UI8_T              null_mac[] = { 0,0,0,0,0,0 };
static AMTR_MGR_PortInfo_T      *amtr_port_info; /*unit(1)+port(1)<=>ifindex(1)<=>amtr_port_info[0]*/
static AMTR_MGR_MacNotifyRec_T  mac_ntfy_rec_lst[AMTR_MGR_MAC_NTFY_LST_MAX];/* queue list for processing */
#define AMTR_PORT_INFO_LOCATE(ifindex)   \
    amtr_port_info[ifindex-1]
#define AMTR_PORT_INFO_VALUE_SET(ifindex,member,value)      \
    do{                                                     \
        IS_IFINDEX_INVALID(ifindex)                         \
        return ;                                            \
      amtr_port_info[ifindex-1].member= value;              \
    }                                                       \
    while(0);
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
static AMTR_TYPE_EventEntry_T   intrusion_mac_buf[SYS_ADPT_AMTR_NBR_OF_ADDR_TO_SYNC_IN_ONE_SCAN];
static AMTR_TYPE_PortMoveEntry_T port_move_buf[SYS_ADPT_AMTR_NBR_OF_ADDR_TO_SYNC_IN_ONE_SCAN];
static AMTR_TYPE_PortMoveEntry_T security_port_move_buf[SYS_ADPT_AMTR_NBR_OF_ADDR_TO_SYNC_IN_ONE_SCAN];
static UI16_T                   port_security_enabled_buf[SYS_ADPT_AMTR_NBR_OF_ADDR_TO_SYNC_IN_ONE_SCAN];
#else
/* buffer for intrusion mac
 */
static AMTR_TYPE_EventEntry_T   intrusion_mac_buf[AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET];
/* buffer for port move callback entry
 */
static AMTR_TYPE_PortMoveEntry_T port_move_buf[AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET];
/* buffer for security port move entry
 */
static AMTR_TYPE_PortMoveEntry_T security_port_move_buf[AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET];
/* buffer for notify "port security enabled because learn_with_count is full" .
 * Every element of this buffer identify a "ifindex"
 */
static UI16_T                   port_security_enabled_buf[AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET];
#endif
/* These two variable will configured by user.
 */
static UI32_T   amtr_mgr_config_aging_time;         /*system-wise*/
static UI32_T   amtr_mgr_config_aging_status;       /*system-wise*/
#if (SYS_CPNT_HASH_LOOKUP_DEPTH_CONFIGURABLE == TRUE)
static UI32_T   amtr_mgr_config_hash_lookup_depth;  /*system-wise*/
#endif

static UI32_T   amtr_mgr_vlan_learning_mode;
static UI32_T   amtr_mgr_operating_mode;
static BOOL_T   amtr_mgr_is_transition_done;

/* Replacement Rule:
 * Invalid < MLAG < Learnt < Security < Config < Internal < Self
 *
 *                                         ( new_entry )
 *                        | internal| invalid | learnt  |   self  | config | security |
 *              -----------------------------------------------------------------------
 *               internal |    T    |    F    |    F    |    T    |    F   |    F     |
 *              -----------------------------------------------------------------------
 *               invalid  |    T    |    T    |    T    |    T    |    T   |    T     |
 * (exist_entry)-----------------------------------------------------------------------
 *               learnt   |    T    |    F    |    T    |    T    |    T   |    T     |
 *              -----------------------------------------------------------------------
 *               self     |    F    |    F    |    F    |    T    |    F   |    F     |
 *              -----------------------------------------------------------------------
 *               config   |    F    |    F    |    F    |    T    |    T   |    F     |
 *              -----------------------------------------------------------------------
 *               security |    F    |    F    |    F    |    T    |    T   |    T     |
 *
 *
 * amtr_mgr_replacement_rule[exist_entry.source-1][new_entry.source-1] =
 * TRUE : new_entry can replace exist_entry.
 * FALSE: new_entry can't replace exist_entry.
 */
const static BOOL_T amtr_mgr_replacement_rule[7][7] =
{
    {TRUE , FALSE, FALSE, TRUE , FALSE, FALSE, FALSE},
    {TRUE , TRUE , TRUE , TRUE , TRUE , TRUE , TRUE },
    {TRUE , FALSE, TRUE , TRUE , TRUE , TRUE , FALSE},
    {FALSE, FALSE, FALSE, TRUE , FALSE, FALSE, FALSE},
    {FALSE, FALSE, FALSE, TRUE , TRUE , FALSE, FALSE},
    {FALSE, FALSE, FALSE, TRUE , TRUE , TRUE , FALSE},
    {TRUE , FALSE, TRUE , TRUE , TRUE , TRUE , TRUE },
};

/* The capacity for address learning
 */
static UI32_T  amtr_mgr_arl_capacity;

#ifdef AMTR_MGR_BACKDOOR_OPEN
static BOOL_T amtr_mgr_bd_print_msg;
static BOOL_T amtr_mgr_bd_perform_test; /*performance testing*/
static UI32_T amtr_mgr_bd_start_learning_tick;
static UI32_T amtr_mgr_bd_start_age_out_tick;
/*ex. amtr_mgr_bd_test_amount=1000. Test how long can AMTR learn 1000 entries.
 */
static UI32_T amtr_mgr_bd_test_amount;
#endif

static L_THREADGRP_Handle_T tg_handle;
static UI32_T               backdoor_member_id;

#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
static AMTR_TYPE_EventEntry_T   edit_entry_buf[SYS_ADPT_AMTR_NBR_OF_ADDR_TO_SYNC_IN_ONE_SCAN];
#endif


/* EXPORTED SUBPROGRAM BODIES
 */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_ProvisionComplete
 * -------------------------------------------------------------------------
 * FUNCTION: This function will tell the Address monitor module to start
 *           action
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void AMTR_MGR_ProvisionComplete(void)
{
    //AMTRDRV_MGR_ProvisionComplete();
    return;
}


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_Init
 *------------------------------------------------------------------------
 * FUNCTION: This function will create and initialize kernel resources
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void AMTR_MGR_Init(void)
{
    /* Init for shared data.
     */
    amtr_port_info = AMTR_OM_GetPortInfoPtr();

    /* Initial Driver Layer
     */
    AMTR_OM_Init();

#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
    AMTR_OM_SetEditAddrEntryBuf(edit_entry_buf, SYS_ADPT_AMTR_NBR_OF_ADDR_TO_SYNC_IN_ONE_SCAN);
#endif

    return;
} /* end AMTR_MGR_Init */

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_Create_InterCSC_Relation
 *------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void AMTR_MGR_Create_InterCSC_Relation(void)
{
#ifdef  AMTR_MGR_BACKDOOR_OPEN
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("AMTR_MGR",
        SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY, AMTR_MGR_BackDoor_Menu);
#endif
}

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_SetTransitionMode()
 *------------------------------------------------------------------------------
 * Purpose  : This function will set the operation mode to transition mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-----------------------------------------------------------------------------*/
void AMTR_MGR_SetTransitionMode(void)
{
    amtr_mgr_is_transition_done = FALSE;
    /* AMTR have to set amtr_mgr_operating_mode first, then send event.
     * Because AMTR_Task check if(operating mode==transition mode) first,
     * then handle transition mode event. */
    AMTR_OM_SetTransitionMode();
    amtr_mgr_operating_mode=SYS_TYPE_STACKING_TRANSITION_MODE;
/*    SYSFUN_SendEvent (amtr_mgr_tid, AMTR_MGR_EVENT_ENTER_TRANSITION_MODE);*/
    return;
}   /*  end of AMTR_MGR_SetTransitionMode   */

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_EnterTransitionMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will initialize all system resource
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void AMTR_MGR_EnterTransitionMode(void)
{
    UI32_T ifindex;

    AMTR_OM_HisamDeleteAllRecord(); /*Clear Hisam Table*/

    /* When enter transition mode, MGR will re-initiate OM. If this thread don't wait Task to finish transition
     * mode. After re-initiating OM, maybe it is set by other thread. In usual, thread have to wait Task to
     * finish transition mode then continue. But AMTR is exceptional case, all record in Hisam table are
     * synchronized from Hash Table. If STKCTRL thread initiate Hash table is earlier than Hisam table,
     * AMTR Task can't get any record from Hash table. Therefore, when AMTR enter transition mode,
     * thread do not wait AMTR task to finish transition mode.
     */
#if 0
    SYSFUN_TASK_ENTER_TRANSITION_MODE(amtr_mgr_is_transition_done);
#endif
    AMTR_OM_EnterTransitionMode();

    /* Initiate constant resource
     */
    amtr_mgr_arl_capacity = SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY;
#if(SYS_CPNT_VXLAN == TRUE)
    for(ifindex=1;ifindex < (SYS_ADPT_TOTAL_NBR_OF_LPORT_INCLUDE_VXLAN+1);ifindex++)
#else
    for(ifindex=1;ifindex < (SYS_ADPT_TOTAL_NBR_OF_LPORT+1);ifindex++)
#endif
    {
        /* set the default value to amtr_port_info
         */
        amtr_port_info[ifindex-1].learn_with_count= SYS_DFLT_PORT_SECURITY_MAX_MAC_COUNT;
        amtr_port_info[ifindex-1].life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT;
        amtr_port_info[ifindex-1].protocol = AMTR_MGR_PROTOCOL_NORMAL;
        amtr_port_info[ifindex-1].is_learn = AMTR_MGR_ENABLE_MACLEARNING_ONPORT;
    }

    amtrmgr_memset(intrusion_mac_buf,0,sizeof(intrusion_mac_buf));
    amtrmgr_memset(port_move_buf,0,sizeof(port_move_buf));
    amtrmgr_memset(security_port_move_buf,0,sizeof(security_port_move_buf));
    amtrmgr_memset(port_security_enabled_buf,0,sizeof(port_security_enabled_buf));
    /* set aging time to Default
     */
    amtr_mgr_config_aging_time = SYS_DFLT_L2_DYNAMIC_ADDR_AGING_TIME;
    amtr_mgr_config_aging_status = SYS_DFLT_L2_DYNAMIC_ADDR_AGING_STATUS;
#if (SYS_CPNT_HASH_LOOKUP_DEPTH_CONFIGURABLE == TRUE)
    amtr_mgr_config_hash_lookup_depth = SYS_DFLT_L2_MAC_TABLE_HASH_LOOKUP_DEPTH;
#endif

    /* set learning mode to IVL
     */
    amtr_mgr_vlan_learning_mode = VAL_dot1qConstraintType_independent;
#ifdef AMTR_MGR_BACKDOOR_OPEN
    amtr_mgr_bd_print_msg=FALSE;
    amtr_mgr_bd_perform_test = FALSE;
    /* When User open the toggle by backdoor, the two variable will be initialized to 0.
     * It means to get system tick.
     */
    amtr_mgr_bd_start_learning_tick =0xFFFF;
    amtr_mgr_bd_start_age_out_tick = 0xFFFF;
#endif

    return;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_EnterMasterMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will enable network monitor services
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Called by stk_ctrl
 *------------------------------------------------------------------------*/
void AMTR_MGR_EnterMasterMode(void)
{
    /*MIKE add for getting SYSTEM_ARL_CAPACITY from stktplg.
      This is future function for mix stacking
     */
    /* amtr_mgr_arl_capacity = STKTPLG_MGR_GetSystemArlCapacity();
     */
    /* set mgr in master mode
     */
    AMTR_OM_EnterMasterMode();

#if (SYS_CPNT_AMTR_VLAN_MAC_LEARNING == TRUE)
    AMTR_MGR_SetVlanLearningDefaultStatus();
#endif
    amtr_mgr_operating_mode=SYS_TYPE_STACKING_MASTER_MODE;

    memset(cpu_mac,0,SYS_ADPT_MAC_ADDR_LEN);

#if (SYS_CPNT_HASH_LOOKUP_DEPTH_CONFIGURABLE == TRUE)
    AMTR_MGR_GetHashLookupDepthFromChip(&amtr_mgr_config_hash_lookup_depth);
#endif

    return;
}


/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_EnterSlaveMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will disable address monitor services
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void AMTR_MGR_EnterSlaveMode(void)
{
    /* set mgr in slave mode
     */
    AMTR_OM_EnterSlaveMode();
    amtr_mgr_operating_mode=SYS_TYPE_STACKING_SLAVE_MODE;
    return;
} /* end AMTR_MGR_Enter_Slave_Mode */


/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_INIT_HandleHotInsertion
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
 * NOTE     : 1.Only one module is inserted at a time.
 * -------------------------------------------------------------------------*/
void AMTR_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    UI32_T  ifindex;
    UI32_T                        hot_insertion_unit;
    UI32_T  total_counter ;

    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        return;
    /*check the parameter value Tony.Lei*/
    if(starting_port_ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT ||number_of_port > SYS_ADPT_TOTAL_NBR_OF_LPORT||(number_of_port + starting_port_ifindex)>= SYS_ADPT_TOTAL_NBR_OF_LPORT)
        return;
    /*check the parameter value Tony.Lei*/
    if((number_of_port + starting_port_ifindex)<= starting_port_ifindex)
        return;

    total_counter = AMTRDRV_OM_GetTotalCounter();
    /* amtr_mgr_arl_capacity = STKTPLG_MGR_GetSystemArlCapacity();
     */
    /* If ARL Table capacity is changed, AMTR have to delete surplus entries
     */
    if (total_counter> amtr_mgr_arl_capacity)
        AMTRDRV_MGR_DelNumOfDynamicAddrs(total_counter-amtr_mgr_arl_capacity);

    /* Have to initial the hot insert port, amtr_port_info table may remain the old information.
     * ex. 1. set 7/1 to secur port. 2.unit7 is removed 3. unit7 is inserted. If AMTR don't re-initial
     *     amtr_port_info[7/1]. The remain value is wrong. Don't need to initial the exist ports.
     */
    for(ifindex = starting_port_ifindex;
        (ifindex<= starting_port_ifindex+number_of_port-1);
        ifindex++ )
    {
        /*
        EPR:       ES3628BT-FLF-ZZ-00347
        Problem:   Port security & Stack: The value of 'max-mac-count' is incorrect on slave
        rootcasue: When HandleHotInsertion it will set max-mac-count as SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY(32*1k) on slave port.
        sloution:  set max-mac-count as SYS_DFLT_PORT_SECURITY_MAX_MAC_COUNT(0) on slave port When HandleHotInsertion.
        File:      amtr_mgr.c
        */
#if 0
        amtr_port_info[ifindex-1].learn_with_count = amtr_mgr_arl_capacity;
#else
        amtr_port_info[ifindex-1].learn_with_count = SYS_DFLT_PORT_SECURITY_MAX_MAC_COUNT;
#endif
        amtr_port_info[ifindex-1].life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT;
        amtr_port_info[ifindex-1].protocol = AMTR_MGR_PROTOCOL_NORMAL;
        amtr_port_info[ifindex-1].is_learn = AMTR_MGR_ENABLE_MACLEARNING_ONPORT;
    }

    /* Reset aging time, if aging time is default value, AMTR don't need to re-set again.
     */
    if (amtr_mgr_config_aging_status==VAL_amtrMacAddrAgingStatus_disabled)
    {
        AMTRDRV_MGR_SetAgingTime(0);
    }
    else if (amtr_mgr_config_aging_time!=SYS_DFLT_L2_DYNAMIC_ADDR_AGING_TIME)
    {
        AMTRDRV_MGR_SetAgingTime(amtr_mgr_config_aging_time);
    }
    hot_insertion_unit = STKTPLG_POM_IFINDEX_TO_DRIVERUNIT(starting_port_ifindex);
    AMTRDRV_MGR_HandleHotInsertion(hot_insertion_unit);
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_INIT_HandleHotRemoval
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1.Only one module is removed at a time.
 * -------------------------------------------------------------------------*/
void AMTR_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    UI32_T ifindex;

    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN();
    if(starting_port_ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT ||number_of_port > SYS_ADPT_TOTAL_NBR_OF_LPORT||(number_of_port + starting_port_ifindex)>= SYS_ADPT_TOTAL_NBR_OF_LPORT)
        AMTR_MGR_RETURN();
    if((number_of_port + starting_port_ifindex)<= starting_port_ifindex)
        AMTR_MGR_RETURN();
    /* get the unit number on which the module is hot swap
     * SWCTRL already remove the module, so use the previous port to get the unit number.
     * (Suppose the number of module is only one.)
     */

    /* MIKE add for getting SYSTEM_ARL_CAPACITY from stktplg.
     * This is future function for mix stacking
     */
    /* amtr_mgr_arl_capacity = STKTPLG_MGR_GetSystemArlCapacity();*/
    for(ifindex = starting_port_ifindex; ifindex<= starting_port_ifindex + number_of_port-1; ifindex++ )
        AMTRDRV_MGR_DeleteAddrByPort(ifindex);
    AMTR_MGR_RETURN();
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetRunningAgeingTime
 *------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          non-default ageing time can be retrieved successfully. Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   None
 * OUTPUT:  *aging_time   - the non-default ageing time
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES:   1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default ageing time.
 *------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AMTR_MGR_GetRunningAgingTime(UI32_T *aging_time)
{
    /* BODY
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (aging_time == NULL)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    *aging_time = amtr_mgr_config_aging_time;

    if (amtr_mgr_config_aging_time == SYS_DFLT_L2_DYNAMIC_ADDR_AGING_TIME)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
} /* End of AMTR_MGR_GetRunningAgeingTime () */

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_SetPortInfo
 *------------------------------------------------------------------------------
 * Purpose  : This function Set the port Infomation
 * INPUT    : ifindex                       - set which port
 *            port_info -> life_time        - When entry is learnt from this port, it's life time
 *            port_info -> count            - The port is learning with the count
 *            port_info -> protocol         - The port is normal port or secur port(port security or dot1X)
 * OUTPUT   : None
 * RETURN   : BOOL_T                        - True : successs, False : failed
 * NOTE     : (refinement; water_huang create(93.8.10))
 *          1. The PortInfo has to be set default value in AMTR_MGR_EnterMasterMode().
 *          2. Other CSCs can use this API to set port information.
 *          3. In this API, AMTR will not check the relationship between inputs.
 *             Callers must set the correct input to this APIs.
 *          4. Iff amtr_port_info[ifindex].protocol is PSEC or DOT1X, AMTR will check the
 *             amtr_port_info[ifindex].learn_with_count when learning.
 *
 *          protocol:   AMTR_MGR_PROTOCOL_NORMAL
 *                      AMTR_MGR_PROTOCOL_PSEC
 *                      AMTR_MGR_PROTOCOL_DOT1X
 *                      AMTR_MGR_PROTOCOL_MAC_AUTH
 *                      AMTR_MGR_PROTOCOL_MAC_AUTH_OR_DOT1X
 *
 *
 *          life_time:  AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT
 *                      AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET
 *                      AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT
 *          count:      if set "0"  - it means no learn !
 *                      else        - learn with the "count"!
 *
 *     coun     |   dyanmic(del on timeout)                     static(!del on timeout)
 *   --------------------------------------------------------------------------------
 *  normal port |   ignore                                      ignore
 *              |
 *  secure port |   SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY   SYS_ADPT_MAX_NBR_OF_STATIC_MAC_PER_PORT
 *              |
 *
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTR_MGR_SetPortInfo(UI32_T ifindex, AMTR_MGR_PortInfo_T *port_info)
{
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;
    SWCTRL_Lport_Type_T     port_type;

    /* Check the current operation mode
     */
    if (amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if(IS_IFINDEX_INVALID_EXCLUDE_0(ifindex)|| ifindex < 1||!port_info)
        AMTR_MGR_RETURN(FALSE);

    if ((amtr_port_info[ifindex-1].learn_with_count == port_info ->learn_with_count)
    &&(amtr_port_info[ifindex-1].life_time == port_info ->life_time)
    &&(amtr_port_info[ifindex-1].protocol == port_info ->protocol))
        AMTR_MGR_RETURN(TRUE);

    /*check input
     */
    if (IS_IFINDEX_INVALID(ifindex)||
        IS_LIFETIME_INVALID(port_info->life_time)||
        IS_PROTOCOL_INVALID(port_info->protocol )||
        (port_info->learn_with_count > amtr_mgr_arl_capacity)||
    /* if life_time of learnt entry is static, AMTR can't learn more
     * than threshold on a port
     */
        (((port_info->life_time < AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)&&
         (port_info->life_time >= AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT))&&
        (port_info->learn_with_count > SYS_ADPT_MAX_NBR_OF_STATIC_MAC_PER_PORT)))
          AMTR_MGR_RETURN(FALSE);

    port_type = SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);

    if ((port_type != SWCTRL_LPORT_NORMAL_PORT) && (port_type != SWCTRL_LPORT_TRUNK_PORT))
        AMTR_MGR_RETURN(FALSE);
    /* Trunk port can't be seure port!
     */
#if (SYS_CPNT_PORT_SECURITY_TRUNK != TRUE)
    /* support port security on trunk port */
    if ((port_type == SWCTRL_LPORT_TRUNK_PORT) &&
        (port_info->protocol!=AMTR_MGR_PROTOCOL_NORMAL))
        AMTR_MGR_RETURN(FALSE);
#endif

    if(amtr_port_info[ifindex-1].life_time != port_info->life_time)
    {
        if(AMTRDRV_MGR_ChangePortLifeTime(ifindex, port_info ->life_time))
            AMTR_OM_HisamUpdateRecordLifeTimeByLPort(ifindex, port_info ->life_time);
    }
    /* Update amtr_port_info table
     */
    amtr_port_info[ifindex-1].learn_with_count = port_info ->learn_with_count;
    amtr_port_info[ifindex-1].life_time = port_info ->life_time;
    amtr_port_info[ifindex-1].protocol = port_info ->protocol;
    AMTR_MGR_RETURN(TRUE);
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_SetAddrEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will set address table entry
 * INPUT :   addr_entry -> vid       - VLAN ID
 *           addr_entry -> mac       - MAC address
 *           addr_entry -> ifindex   - interface index
 *           addr_entry -> action    - Forwarding/Discard by SA match/Discard by DA match/Trap to CPU only/Forwarding and trap to CPU
 *           addr_entry -> source    - Config/Learn/Internal/Self/Security
 *           addr_entry -> life_time - permanent/Other/Delete on Reset/Delete on Timeout
 * OUTPUT  : None
 * RETURN  : BOOL_T status           - Success or not
 * NOTE    :
 *           1.parameter:
 *             action:    AMTR_TYPE_ADDRESS_ACTION_FORWARDING
 *                        AMTRDRV_OM_ADDRESS_ACTION_DISCARDBYSAMATCH
 *                        AMTRDRV_OM_ADDRESS_ACTION_DISCARDBYDAMATCH
 *                        AMTRDRV_OM_ADDRESS_ACTION_TRAPTOCPUONLY
 *                        AMTR_TYPE_ADDRESS_ACTION_FORWARDINGANDTRAPTOCPU
 *
 *             source:    AMTR_TYPE_ADDRESS_SOURCE_LEARN
 *                        AMTR_TYPE_ADDRESS_SOURCE_CONFIG
 *                        AMTR_TYPE_ADDRESS_SOURCE_INTERNAL
 *                        AMTR_TYPE_ADDRESS_SOURCE_SELF
 *                        AMTR_TYPE_ADDRESS_SOURCE_SECURITY
 *
 *             life_time: AMTR_TYPE_ADDRESS_LIFETIME_OTHER
 *                        AMTR_TYPE_ADDRESS_LIFETIME_INVALID
 *                        AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT
 *                        AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET
 *                        AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT
 *           2.Set CPU mac don't care the ifinedx, but can't be "0".
 *             In AMTR, ifindex==0 means just set to OM, don't program chip.
 *------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_SetAddrEntry( AMTR_TYPE_AddrEntry_T *addr_entry)
{
    BOOL_T ret_val=FALSE;

    /* BODY
     */
    if(!addr_entry)
        AMTR_MGR_RETURN(ret_val);

    /* for voice vlan, a new field 'priority' is added to addr_entry.
     * to be compatible with original function,
     * set priority to AMTR_TYPE_ADDRESS_WITHOUT_PRIORITY before
     * calling AMTRDRV.
     */
    addr_entry->priority = AMTR_TYPE_ADDRESS_WITHOUT_PRIORITY;

    ret_val = AMTR_MGR_SetAddrEntryWithPriority(addr_entry);

    AMTR_MGR_RETURN(ret_val);
}

#if (SYS_CPNT_MLAG == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_SetAddrEntryForMlag
 *------------------------------------------------------------------------
 * FUNCTION: This function will set address table entry
 * INPUT :   addr_entry -> vid       - VLAN ID
 *           addr_entry -> mac       - MAC address
 *           addr_entry -> ifindex   - interface index
 *           addr_entry -> action    - Forwarding/Discard by SA match/Discard by DA match/Trap to CPU only/Forwarding and trap to CPU
 *           addr_entry -> source    - Config/Learn/Internal/Self/Security
 *           addr_entry -> life_time - permanent/Other/Delete on Reset/Delete on Timeout
 * OUTPUT  : None
 * RETURN  : BOOL_T status           - Success or not
 * NOTE    :
 *           1.parameter:
 *             action:    AMTR_TYPE_ADDRESS_ACTION_FORWARDING
 *                        AMTRDRV_OM_ADDRESS_ACTION_DISCARDBYSAMATCH
 *                        AMTRDRV_OM_ADDRESS_ACTION_DISCARDBYDAMATCH
 *                        AMTRDRV_OM_ADDRESS_ACTION_TRAPTOCPUONLY
 *                        AMTR_TYPE_ADDRESS_ACTION_FORWARDINGANDTRAPTOCPU
 *
 *             source:    AMTR_TYPE_ADDRESS_SOURCE_LEARN
 *                        AMTR_TYPE_ADDRESS_SOURCE_CONFIG
 *                        AMTR_TYPE_ADDRESS_SOURCE_INTERNAL
 *                        AMTR_TYPE_ADDRESS_SOURCE_SELF
 *                        AMTR_TYPE_ADDRESS_SOURCE_SECURITY
 *
 *             life_time: AMTR_TYPE_ADDRESS_LIFETIME_OTHER
 *                        AMTR_TYPE_ADDRESS_LIFETIME_INVALID
 *                        AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT
 *                        AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET
 *                        AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT
 *           2.Set CPU mac don't care the ifinedx, but can't be "0".
 *             In AMTR, ifindex==0 means just set to OM, don't program chip.
 *           3.The mac entry which is added through this function will not be
 *             notified to MLAG, and this function shall only be called by MLAG.
 *------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_SetAddrEntryForMlag( AMTR_TYPE_AddrEntry_T *addr_entry)
{
    BOOL_T ret_val=FALSE;

    /* BODY
     */
    if(!addr_entry)
        AMTR_MGR_RETURN(ret_val);

    /* for voice vlan, a new field 'priority' is added to addr_entry.
     * to be compatible with original function,
     * set priority to AMTR_TYPE_ADDRESS_WITHOUT_PRIORITY before
     * calling AMTRDRV.
     */
    addr_entry->priority = AMTR_TYPE_ADDRESS_WITHOUT_PRIORITY;

    ret_val = AMTR_MGR_SetAddrEntryWithPriorityAndMacNotify(addr_entry, AMTR_MGR_MAC_NTFY_MLAG);

    AMTR_MGR_RETURN(ret_val);
}
#endif

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_SetAddrEntryWithPriority
 *------------------------------------------------------------------------
 * FUNCTION: This function will set address table entry
 * INPUT :   addr_entry -> vid       - VLAN ID
 *           addr_entry -> mac       - MAC address
 *           addr_entry -> ifindex   - interface index
 *           addr_entry -> action    - Forwarding/Discard by SA match/Discard by DA match/Trap to CPU only/Forwarding and trap to CPU
 *           addr_entry -> source    - Config/Learn/Internal/Self/Security
 *           addr_entry -> life_time - permanent/Other/Delete on Reset/Delete on Timeout
 *           addr_entry -> priority  - QoS priority
 * OUTPUT  : None
 * RETURN  : BOOL_T status           - Success or not
 * NOTE    : 1. parameters:
 *              action:     AMTRDRV_OM_ADDRESS_ACTION_FORWARDING
 *                          AMTRDRV_OM_ADDRESS_ACTION_DISCARDBYSAMATCH
 *                          AMTRDRV_OM_ADDRESS_ACTION_DISCARDBYDAMATCH
 *                          AMTRDRV_OM_ADDRESS_ACTION_TRAPTOCPUONLY
 *                          AMTRDRV_OM_ADDRESS_ACTION_FORWARDINGANDTRAPTOCPU
 *
 *              source:     AMTRDRV_OM_ADDRESS_SOURCE_LEARN
 *                          AMTRDRV_OM_ADDRESS_SOURCE_CONFIG
 *                          AMTRDRV_OM_ADDRESS_SOURCE_INTERNAL
 *                          AMTRDRV_OM_ADDRESS_SOURCE_SELF
 *                          AMTR_TYPE_ADDRESS_SOURCE_SECURITY
 *
 *              life_time:  AMTRDRV_OM_ADDRESS_LIFETIME_OTHER
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_INVALID
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_PERMANENT
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_DELETEONRESET
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_DELETEONTIMEOUT
 *           2.Set CPU mac don't care the ifinedx, but can't be "0".
 *           In AMTR, ifindex ==0 means just set to OM, don't program chip.
 *------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_SetAddrEntryWithPriority(AMTR_TYPE_AddrEntry_T *addr_entry)
{
    return AMTR_MGR_SetAddrEntryWithPriorityAndMacNotify(addr_entry, AMTR_MGR_MAC_NTFY_NONE);
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_SetAddrEntryWithPriorityAndMacNotify
 *------------------------------------------------------------------------
 * FUNCTION: This function will set address table entry
 * INPUT :   addr_entry -> vid       - VLAN ID
 *           addr_entry -> mac       - MAC address
 *           addr_entry -> ifindex   - interface index
 *           addr_entry -> action    - Forwarding/Discard by SA match/Discard by DA match/Trap to CPU only/Forwarding and trap to CPU
 *           addr_entry -> source    - Config/Learn/Internal/Self/Security
 *           addr_entry -> life_time - permanent/Other/Delete on Reset/Delete on Timeout
 *           addr_entry -> priority  - QoS priority
 *           skip_mac_notify         - decide which component does not need to notify
 * OUTPUT  : None
 * RETURN  : BOOL_T status           - Success or not
 * NOTE    : 1. parameters:
 *              action:     AMTRDRV_OM_ADDRESS_ACTION_FORWARDING
 *                          AMTRDRV_OM_ADDRESS_ACTION_DISCARDBYSAMATCH
 *                          AMTRDRV_OM_ADDRESS_ACTION_DISCARDBYDAMATCH
 *                          AMTRDRV_OM_ADDRESS_ACTION_TRAPTOCPUONLY
 *                          AMTRDRV_OM_ADDRESS_ACTION_FORWARDINGANDTRAPTOCPU
 *
 *              source:     AMTRDRV_OM_ADDRESS_SOURCE_LEARN
 *                          AMTRDRV_OM_ADDRESS_SOURCE_CONFIG
 *                          AMTRDRV_OM_ADDRESS_SOURCE_INTERNAL
 *                          AMTRDRV_OM_ADDRESS_SOURCE_SELF
 *                          AMTR_TYPE_ADDRESS_SOURCE_SECURITY
 *
 *              life_time:  AMTRDRV_OM_ADDRESS_LIFETIME_OTHER
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_INVALID
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_PERMANENT
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_DELETEONRESET
 *                          AMTRDRV_OM_ADDRESS_LIFETIME_DELETEONTIMEOUT
 *           2.Set CPU mac don't care the ifinedx, but can't be "0".
 *           In AMTR, ifindex ==0 means just set to OM, don't program chip.
 *------------------------------------------------------------------------*/
static BOOL_T AMTR_MGR_SetAddrEntryWithPriorityAndMacNotify(AMTR_TYPE_AddrEntry_T *addr_entry, UI8_T skip_mac_notify)
{
    AMTRDRV_TYPE_Record_T   get_entry;
    /* we use "SWCTRL_LogicalPortToUserPort" to check the port type of ifindex.
     * This API will return unit, port and trunk_id, so we must declare "temp_unit",
     * "temp_port" and "temp_trunk_id". These three variables just for receive the output.
     */
    SWCTRL_Lport_Type_T type;
    UI32_T              temp_unit;
    UI32_T              temp_port;
    UI32_T              temp_trunk_id;
    BOOL_T              is_entry_exist_om=FALSE;
    Port_Info_T         p_info;         /* get port information from swctrl*/

    /* BODY
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);
    if(!addr_entry)
        AMTR_MGR_RETURN(FALSE);

#if (SYS_CPNT_VXLAN == TRUE)
    if(AMTR_TYPE_IS_REAL_VFI(addr_entry->vid))
    {
        UI16_T l_vfi=0;

        if(AMTRDRV_OM_GetAndAlocRVfiMapToLVfi(addr_entry->vid, &l_vfi)==FALSE)
        {
            AMTR_MGR_RETURN(FALSE);
        }
        addr_entry->vid=l_vfi;
    }
#endif

    get_entry.address.vid = addr_entry->vid;
    amtrmgr_memcpy(get_entry.address.mac, addr_entry->mac, AMTR_TYPE_MAC_LEN);

    /* check the entry is exist in OM or not
     */
    if(AMTRDRV_OM_GetExactRecord( (UI8_T *)&get_entry))
    {
        /* check the new entry and old entry is the same or not
         */
        if(!amtrmgr_memcmp(addr_entry, &get_entry.address,sizeof(AMTR_TYPE_AddrEntry_T)))
            AMTR_MGR_RETURN(TRUE);
        is_entry_exist_om=TRUE;
    }

    /* Check input
     */

    /* Only process entry and config entries
     */
    if (((addr_entry->source!=AMTR_TYPE_ADDRESS_SOURCE_CONFIG)&&
        (addr_entry->source!=AMTR_TYPE_ADDRESS_SOURCE_LEARN)&&
        (addr_entry->source!=AMTR_TYPE_ADDRESS_SOURCE_SECURITY)&&
        (addr_entry->source!=AMTR_TYPE_ADDRESS_SOURCE_MLAG))||
        (IS_LIFETIME_INVALID(addr_entry->life_time))||
        (IS_ACTION_INVALID( addr_entry->action))||
        (IS_MULTICAST_MAC(addr_entry->mac) || IS_MAC_INVALID(addr_entry->mac)) ||
        ( IS_VID_INVALID(addr_entry->vid))||
        ( IS_IFINDEX_INVALID_EXCLUDE_0(addr_entry->ifindex))) /* ifindex == 0 is legal.*/
            AMTR_MGR_RETURN(FALSE);

    /* If AMTR call other CSC's API for checking, it have to get semaphore first.
     * That can avoid "race condition".
     * For example: When AMTRDRV Task learn NA, it will call this API to set entry.
     * AMTR(AMTRDRV Task) will check whether port admin disable or not before call AMTRDRV.
     * The steps are 1. call swctrl 2. get semaphore 3. call AMTRDRV.
     * If user config port admin disable after step1. And UI priority is higher than AMTR.
     * AMTRDRV Task have to wait semaphore until del_dyn_by_port finished.(UI Task do del_dyn_by_port)
     * When AMTRDRV Task get semaphore, the port admin is disabled at this moment.
     * AMTRDRV Task shouldn't this NA to OM because port admin is disable.
     */
#ifdef SYS_HWCFG_NO_VID_FIELD_IN_ADDR_TBL
    if (addr_entry->vid != 1)
        AMTR_MGR_RETURN(FALSE);
#else
    /* check the vlan is exist
     */
        if (!AMTR_MGR_CHECK_VLAN_VALIDITY(addr_entry->vid))
            AMTR_MGR_RETURN(FALSE);
#endif

#if defined(SYS_ADPT_MGMT_PORT)
    if (SYS_ADPT_MGMT_PORT == addr_entry->ifindex)
        AMTR_MGR_RETURN(FALSE);
#endif

    if(is_entry_exist_om)
    {
        /* Can new_entry replace exist_entry or not ?
         */
        if (amtr_mgr_replacement_rule[get_entry.address.source-1][addr_entry->source-1]==FALSE)
            AMTR_MGR_RETURN(FALSE);
    }

    /*
     * AMTR_MGR_Notify_SetStaticMACCheck()
     * Added by Merlin, 2006-03-08
     *
     * - AMTR_MGR_Notify_SetStaticMACCheck() will check if the static MAC is to be set.
     * - Only when AMTR_MGR_Notify_SetStaticMACCheck() return TRUE, the static MAC
     *   address will be set (add/delete).
     */
    if (addr_entry->action == AMTR_TYPE_ADDRESS_ACTION_FORWARDING &&
        addr_entry->source == AMTR_TYPE_ADDRESS_SOURCE_CONFIG &&
        addr_entry->life_time == AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT)
    {
        if (!AMTR_MGR_Notify_SetStaticMACCheck(addr_entry->vid, addr_entry->mac, addr_entry->ifindex))
            AMTR_MGR_RETURN(FALSE);
    }

    if ((addr_entry->life_time == AMTR_TYPE_ADDRESS_LIFETIME_OTHER)||(addr_entry->ifindex==0))
    {
        /* Delete old entry directly from chip, hash table and hisam table.
         * (either Master or Slave)
         */
        if (is_entry_exist_om)
        {
            if (AMTRDRV_MGR_DeleteAddrDirectly(&(get_entry.address), FALSE)==FALSE)
                AMTR_MGR_RETURN(FALSE);
        }

        /* "other" entry is only set in master's hash table and hisam table.
         */
        if (AMTRDRV_MGR_SetAddrDirectly(addr_entry,TRUE))
        {
            AMTR_MGR_RETURN(TRUE);
        }
        else
        {
            AMTR_MGR_RETURN(FALSE);
        }
    }

    type = SWCTRL_LogicalPortToUserPort(addr_entry->ifindex, &temp_unit, &temp_port, &temp_trunk_id);
    /* User can't set a entry with trunk member or unknow port
     */
    if ((type != SWCTRL_LPORT_NORMAL_PORT) && (type != SWCTRL_LPORT_TRUNK_PORT)
#if(SYS_CPNT_VXLAN == TRUE)
        && (type != SWCTRL_LPORT_VXLAN_PORT)
#endif
       )
        AMTR_MGR_RETURN(FALSE);
         /* mac can not insert into trunk member port */

/* Config or internal entry have to be set in OM even if table is full.
 */
#if 0
    /*check system total count is full ?*/
    if (AMTRDRV_OM_GetTotalCounter() >= amtr_mgr_arl_capacity)
    {
        printf("ERROR15\r\n");
        return FALSE;
    }
#endif

    if (AMTRDRV_OM_GetTotalCounter() >= amtr_mgr_arl_capacity)
        AMTR_MGR_RETURN(FALSE);

    /* check system static count is full ?
     * static counter= tatol counter - dynamic counter;
     */
    if((addr_entry->life_time != AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT) &&
      ((AMTRDRV_OM_GetTotalStaticCounter()>= SYS_ADPT_MAX_NBR_TOTAL_STATIC_MAC)||
       (AMTRDRV_OM_GetStaticCounterByPort(addr_entry->ifindex)>=SYS_ADPT_MAX_NBR_OF_STATIC_MAC_PER_PORT)))
            AMTR_MGR_RETURN(FALSE);

    /* if learnt_with_count include user config entries, AMTR have to check
     * whether learnt_with_count is full or not before set config entry.
     */
#if (SYS_DFLT_L2_ADDR_PSEC_LEARN_COUNT_INCLUDE_CONFIG)
    if ((addr_entry->source == AMTR_TYPE_ADDRESS_SOURCE_CONFIG)&&
        ( AMTR_MGR_IsPortSecurityEnabled(addr_entry->ifindex)))
    {
        AMTR_MGR_Notify_IntrusionMac(addr_entry->ifindex,addr_entry->vid,addr_entry->mac, (UI8_T *)null_mac,0);
        AMTR_MGR_RETURN(FALSE);
    }
#endif

    /* Security CSC like PSec or Dot1X CSCs will set learnt entry by this
     * API,and will set addr_entry->source as AMTR_TYPE_ADDRSSS_SOURCE_SECURITY
     */
    if (addr_entry->source == AMTR_TYPE_ADDRESS_SOURCE_SECURITY)
    {   /* only learnt entry need to check these states
         */
        if ((!SWCTRL_GetPortInfo(addr_entry->ifindex, &p_info))||
        /* check port admin status and link status
         */
        ((p_info.admin_state == VAL_ifAdminStatus_down)||(p_info.link_status == SWCTRL_LINK_DOWN))||
        /* check port spanning tree state
         */
        (!AMTR_MGR_CHECK_STP_STATE_VALIDITY(addr_entry->vid, addr_entry->ifindex)))
        {
            AMTR_MGR_RETURN(FALSE);
            /* mac can not insert into trunk member port */
        }

        /* don't need to check intrusion mac or port move ,
         * Psec or Dot1X should do checking by itself before set learnt entry.
         * ex. PSEC or Dot1X should check 1. intrusion mac 2. port mvoe.
         */
#if 0
        if (AMTR_MGR_IsPortSecurityEnabled(addr_entry->ifindex))
        {
            AMTR_MGR_Notify_IntrusionMac(addr_entry->ifindex, addr_entry->vid, addr_entry->mac);
            return FALSE;
        }


        if (is_entry_exist_om)
        {
            if (addr_entry->ifindex != get_entry.address.ifindex)       /*this is port move*/
            {   /*1. delete the old entry*/
                retval=AMTRDRV_MGR_DeleteAddrEntryList(1, &get_entry);
                /*2. notify upper CSC*/
                if (amtr_port_info[(get_entry.address.ifindex)-1].protocol!=AMTR_MGR_PROTOCOL_NORMAL)
                    AMTR_MGR_Notify_SecurityPortMove(addr_entry->ifindex, addr_entry->vid, addr_entry->mac, get_entry.address.ifindex);

                return retval;
            }
        }
#endif
    }

    /* replace = delete + add, this is for maintain counters.
     */
    if (is_entry_exist_om)
    {
        /* don't need to care "self" entry, it can't run this procedure.
         */
        if ((get_entry.address.life_time== AMTR_TYPE_ADDRESS_LIFETIME_OTHER)||(get_entry.address.ifindex==0))
        {
            AMTRDRV_MGR_DeleteAddrDirectly(&(get_entry.address), FALSE);
        }
        else
        {
        #if (AMTR_TYPE_DEBUG_PACKET_FLOW == 1)
            if(AMTR_MGR_IS_TRACED_INTERFACE(get_entry.address.ifindex))
                BACKDOOR_MGR_Printf("%s-%d port move from index%i to index%i, vid%i, mac %0x:%0x:%0x:%0x:%0x:%0x\r\n", __FUNCTION__, __LINE__, get_entry.address.ifindex, addr_entry->ifindex, get_entry.address.vid, get_entry.address.mac[0], get_entry.address.mac[1], get_entry.address.mac[2], get_entry.address.mac[3], get_entry.address.mac[4], get_entry.address.mac[5]);
        #endif
            AMTR_MGR_MacNotifyAddNewEntry(get_entry.address.ifindex, get_entry.address.vid, get_entry.address.mac, FALSE, skip_mac_notify);
            AMTRDRV_MGR_DeleteAddrEntryList(1, &(get_entry.address));
        }
    }

    if(AMTRDRV_MGR_SetAddrList(1, addr_entry))
    {
    #if (AMTR_TYPE_DEBUG_PACKET_FLOW == 1)
        if(AMTR_MGR_IS_TRACED_INTERFACE(addr_entry->ifindex))
            BACKDOOR_MGR_Printf("%s-%d the NA is index%i, vid%i, mac %0x:%0x:%0x:%0x:%0x:%0x\r\n", __FUNCTION__, __LINE__, addr_entry->ifindex, addr_entry->vid, addr_entry->mac[0], addr_entry->mac[1], addr_entry->mac[2], addr_entry->mac[3], addr_entry->mac[4], addr_entry->mac[5]);
    #endif
        AMTR_MGR_MacNotifyAddNewEntry(addr_entry->ifindex, addr_entry->vid, addr_entry->mac, TRUE, skip_mac_notify);

        /* AMTR only notify "auto learn counter is full(=port security enabled)" once.
         *  If next config entry is incoming, it will be dropped because of port seurity enabled.
         */
        /* Learnt entry don't need to check AMTR_MGR_IsPortSecurityEnabled().
         * Because AMTR_MGR_IsPortSecurityEnabled() alwarys == TRUE.
         * Security CSC like PSec or Dot1X will set learnt entry by this API,
         * and will set addr_entry->source as AMTR_TYPE_ADDRSSS_SOURCE_SECURITY.
         * AMTR trap learnt entry to PSec or Dot1X if port security is enabled.
         */
#if (SYS_DFLT_L2_ADDR_PSEC_LEARN_COUNT_INCLUDE_CONFIG)
        if ((addr_entry->source==AMTR_TYPE_ADDRESS_SOURCE_CONFIG) /*learn_with_count include learnt and config entry*/
        /* Procedure should in critical section when call AMTR_MGR_IsPortSecurityEnabled()
         */
            &&( AMTR_MGR_IsPortSecurityEnabled(addr_entry->ifindex)))
        {
            AMTR_MGR_Notify_AutoLearnCount(addr_entry->ifindex, VAL_portSecPortStatus_enabled);
            AMTR_MGR_RETURN(TRUE);
        }
#endif
        AMTR_MGR_RETURN(TRUE);
    }
    else
    {
        AMTR_MGR_RETURN(FALSE);
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddr
 * -------------------------------------------------------------------------
 * FUNCTION: This function will clear specific address table entry (either
 *           dynamic or static)
 * INPUT   : UI16_T  vid   - vlan ID
 *           UI8_T  *Mac   - Specified MAC to be cleared
 *
 * OUTPUT  : None
 * RETURN  : BOOL_T Status - True : successs, False : failed
 * NOTE:     This API can't delete CPU MAC!
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddr(UI32_T vid, UI8_T *mac)
{
    return AMTR_MGR_DeleteAddrWithMacNotify(vid, mac, AMTR_MGR_MAC_NTFY_NONE);
}

#if (SYS_CPNT_MLAG == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrForMlag
 * -------------------------------------------------------------------------
 * FUNCTION: This function will clear specific address table entry (either
 *           dynamic or static)
 * INPUT   : UI16_T  vid   - vlan ID
 *           UI8_T  *Mac   - Specified MAC to be cleared
 *
 * OUTPUT  : None
 * RETURN  : BOOL_T Status - True : successs, False : failed
 * NOTE:     1.This API can't delete CPU MAC!
 *           2.The mac entry which is deleted through this function will not be
 *             notified to MLAG, and this function shall only be called by MLAG.
 * -------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_DeleteAddrForMlag(UI32_T vid, UI8_T *mac)
{
    return AMTR_MGR_DeleteAddrWithMacNotify(vid, mac, AMTR_MGR_MAC_NTFY_MLAG);
}
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrWithMacNotify
 * -------------------------------------------------------------------------
 * FUNCTION: This function will clear specific address table entry (either
 *           dynamic or static)
 * INPUT   : UI16_T  vid   - vlan ID
 *           UI8_T  *Mac   - Specified MAC to be cleared
 *           UI8_T  skip_mac_notify - decide which component does not need to notify
 *
 * OUTPUT  : None
 * RETURN  : BOOL_T Status - True : successs, False : failed
 * NOTE:     This API can't delete CPU MAC!
 * -------------------------------------------------------------------------
 */
static BOOL_T AMTR_MGR_DeleteAddrWithMacNotify(UI32_T vid, UI8_T *mac, UI8_T skip_mac_notify)
{
    AMTRDRV_TYPE_Record_T addr_entry;
    BOOL_T              retval = FALSE;
    /* BODY
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

#if (SYS_CPNT_VXLAN == TRUE)
    if(AMTR_TYPE_IS_REAL_VFI(vid))
    {
        UI16_T l_vfi = 0;

        if(AMTRDRV_OM_GetAndAlocRVfiMapToLVfi(vid, &l_vfi)==FALSE)
        {
            AMTR_MGR_RETURN(FALSE);
        }
        vid = l_vfi;
    }
#endif

    if (IS_MAC_INVALID(mac)||IS_VID_INVALID(vid ))
        AMTR_MGR_RETURN(FALSE);

    addr_entry.address.vid = vid;
    amtrmgr_memcpy(addr_entry.address.mac, mac, AMTR_TYPE_MAC_LEN);

    if (AMTRDRV_OM_GetExactRecord((UI8_T *)&addr_entry) == FALSE)
    {
        /* can't get == not exist == already be deleted, return TRUE
         */
        AMTR_MGR_RETURN(TRUE);
    }
    if (addr_entry.address.source==AMTR_TYPE_ADDRESS_SOURCE_SELF)
        AMTR_MGR_RETURN(FALSE);

    if ((addr_entry.address.life_time == AMTR_TYPE_ADDRESS_LIFETIME_OTHER)||(addr_entry.address.ifindex ==0))
        retval = AMTRDRV_MGR_DeleteAddrDirectly(&addr_entry.address,TRUE);
    else
    {
        retval=AMTRDRV_MGR_DeleteAddrEntryList(1, &addr_entry.address);
    }
    if (retval)
    {
        AMTR_MGR_UpdatePortSecurityStatus(addr_entry.address.ifindex);
    }

    AMTR_MGR_Notify_EditAddrEntry(vid, mac, addr_entry.address.ifindex, TRUE);
    if(addr_entry.address.life_time == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)
    {
        AMTR_MGR_MacNotifyAddNewEntry(addr_entry.address.ifindex, vid, mac, FALSE, skip_mac_notify);
    }

    AMTR_MGR_RETURN(retval);
} /* end AMTR_MGR_DeleteAddrWithMacNotify */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrByLPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will clear all address table entries
 *           by port
 * INPUT   : UI32_T ifindex - interface index
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * Note    : deletion in both chip and amtr module
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddrByLPort(UI32_T ifindex)
{
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;
    SWCTRL_Lport_Type_T     port_type;
    BOOL_T                  retval = FALSE;
    /* BODY
     */
    /* Check the current operation mode
     */
    if (amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    /* Validate the given parameter
     */
    if (IS_IFINDEX_INVALID(ifindex))
        AMTR_MGR_RETURN(FALSE);

    port_type = SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);

    if ((port_type != SWCTRL_LPORT_NORMAL_PORT)&&(port_type != SWCTRL_LPORT_TRUNK_PORT)
#if (SYS_CPNT_VXLAN == TRUE)
        &&(port_type != SWCTRL_LPORT_VXLAN_PORT)
#endif
        )
        AMTR_MGR_RETURN(FALSE);

    retval= AMTRDRV_MGR_DeleteAddrByPort(ifindex);
    if (retval)
        AMTR_MGR_Notify_MACTableDeleteByPort(ifindex, AMTR_MGR_UNKNOWN);
    AMTR_MGR_RETURN(retval);
} /* end AMTR_MGR_DeleteAddr_ByPort */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrByVID
 * -------------------------------------------------------------------------
 * FUNCTION: This function will clear dynamic & static address table entries by vid
 * INPUT   : UI32_T vid     - vlan id
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    : deletion in both chip and amtr module
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddrByVID(UI32_T vid)
{

   BOOL_T retval=FALSE;
#ifdef SYS_HWCFG_NO_VID_FIELD_IN_ADDR_TBL
   AMTR_MGR_RETURN(FALSE);
#else

    /* BODY
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    /* Only support logical vid which belongs to dot1q vlan id
     */
    if(!AMTR_TYPE_LOGICAL_VID_IS_DOT1Q_VID(vid))
        AMTR_MGR_RETURN(FALSE);

    retval=AMTRDRV_MGR_DeleteAddrByVID(vid);

    if (retval)
        AMTR_MGR_Notify_MACTableDeleteByVid(vid);
    AMTR_MGR_RETURN(retval);
#endif
} /* end AMTR_MGR_DeleteAddr_byVID */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrByLifeTime
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete entries by life time
 * INPUT   : AMTR_TYPE_AddressLifeTime_T life_time
 * OUTPUT  : None
 * RETURN  : BOOL_T Status - True : successs, False : failed
 * NOTE    : deletion in both chip and amtr module
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddrByLifeTime(AMTR_TYPE_AddressLifeTime_T life_time)
{
    BOOL_T  retval=FALSE;
    /* BODY
     */
    if (amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(retval);

    /* check input
    */
    if (IS_LIFETIME_INVALID(life_time))
        AMTR_MGR_RETURN(retval);

    retval=AMTRDRV_MGR_DeleteAddrByLifeTime(life_time);

    if (retval)
        AMTR_MGR_Notify_MacTableDeleteByLifeTime(life_time);

    AMTR_MGR_RETURN(retval);
} /* end AMTR_MGR_DeleteAllDynamicAddr */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrBySource
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete entries by source from OM
 * INPUT   : AMTR_TYPE_AddressSource_T source
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T     AMTR_MGR_DeleteAddrBySource(AMTR_TYPE_AddressSource_T source)
{
    BOOL_T  retval=FALSE;
    /* BODY
     */
    if (amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(retval);
    /* check input
     */
    if (IS_SOURCE_INVALID(source))
        AMTR_MGR_RETURN(retval);

    retval=AMTRDRV_MGR_DeleteAddrBySource(source);

    AMTR_MGR_RETURN(retval);
} /* end AMTR_MGR_DeleteDynamicAddr_byVID */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrByVidAndLPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by vid and Port.
 * INPUT   : UI32_T ifindex - interface index
 *           UI32_T vid - VLAN ID
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddrByVidAndLPort(UI32_T ifindex, UI32_T vid)
{
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;
    SWCTRL_Lport_Type_T     port_type;
    BOOL_T                  retval=FALSE;
    UI32_T                  action_count=0; /*How many entries are deleted in Hisam*/
    /* BODY
     */

    /* Check the current operation mode
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(retval);

    /* Validate the given parameter
     * Only support logical vid which belongs to dot1q vlan id
     */
    if(IS_IFINDEX_INVALID(ifindex)
#if (SYS_CPNT_VXLAN == TRUE)
       ||(!AMTR_TYPE_IS_VALID_VID_OR_REAL_VFI(vid)))
#else
       ||(!AMTR_TYPE_LOGICAL_VID_IS_DOT1Q_VID(vid)))
#endif
        AMTR_MGR_RETURN(retval);

    port_type = SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);

    if ((port_type != SWCTRL_LPORT_NORMAL_PORT)&&(port_type != SWCTRL_LPORT_TRUNK_PORT)
#if (SYS_CPNT_VXLAN == TRUE)
        &&(port_type != SWCTRL_LPORT_VXLAN_PORT)
#endif
        )
        AMTR_MGR_RETURN(retval);
    /* del_by_port+vid+life_time, AMTRDRV won't sync to AMTR_MGR. Core layer
     * have to update hisam table by itself.
     */
    if ((retval=AMTRDRV_MGR_DeleteAddrByVIDnPort(ifindex,vid)))
    {
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
        BOOL_T do_again;
        do {
#endif
        AMTR_OM_HisamDeleteRecordByLPortAndVid(ifindex,vid, &action_count);
        AMTR_MGR_UpdatePortSecurityStatus(ifindex);
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
        AMTR_MGR_NOTIFY_EDIT_ADDR_ENTRY_FROM_BUFFER(action_count, TRUE, &do_again);
        } while (do_again);
#endif
    }
    if (retval)
        AMTR_MGR_Notify_MACTableDeleteByVIDnPort(vid, ifindex);

    AMTR_MGR_RETURN(retval);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrByLifeTimeAndLPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by Life Time and Port.
 * INPUT   : UI32_T ifindex - interface index
 *           AMTR_MGR_LiftTimeMode_T life_time - other,invalid, delete on timeout, delete on reset, permanent
 * OUTPUT  : None
 * RETURN  : BOOL_T Status                     - True : successs, False : failed
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddrByLifeTimeAndLPort(UI32_T ifindex, AMTR_TYPE_AddressLifeTime_T life_time)
{
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;
    SWCTRL_Lport_Type_T     port_type;
    BOOL_T                  retval=FALSE;
    /* BODY
     */

    /* Check the current operation mode
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(retval);

    /* Validate the given parameter
     */
    if(IS_IFINDEX_INVALID(ifindex)||IS_LIFETIME_INVALID(life_time))
        AMTR_MGR_RETURN(retval);

    /* If the ifindex is secure port, the life time of entries of this
     * port will be specific life_time
     */
    if((amtr_port_info[ifindex-1].protocol!=AMTR_MGR_PROTOCOL_NORMAL)
     &&(amtr_port_info[ifindex-1].life_time!=life_time))
        AMTR_MGR_RETURN(TRUE);

    port_type = SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);

   if ((port_type != SWCTRL_LPORT_NORMAL_PORT)&&(port_type != SWCTRL_LPORT_TRUNK_PORT)
#if (SYS_CPNT_VXLAN == TRUE)
    &&(port_type != SWCTRL_LPORT_VXLAN_PORT)
#endif
    )
       AMTR_MGR_RETURN(retval);

    retval = AMTRDRV_MGR_DeleteAddrByPortAndLifeTime(ifindex, life_time);

    if (retval)
        AMTR_MGR_Notify_MACTableDeleteByPort(ifindex,  AMTR_MGR_UNKNOWN);

    AMTR_MGR_RETURN(retval);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrBySourceAndLPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by specific source and Lport.
 * INPUT   : UI32_T ifindex               - interface index
 *           AMTR_MGR_SourceMode_T source - learnt, config, intrenal, self
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    : deletion in both chip and amtr module
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddrBySourceAndLPort(UI32_T ifindex, AMTR_TYPE_AddressSource_T source)
{
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;
    BOOL_T                  retval=FALSE;
    SWCTRL_Lport_Type_T     port_type;
    /* BODY
     */

    /* Check the current operation mode
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    /* Validate the given parameter
     */
    if(IS_IFINDEX_INVALID(ifindex)||IS_SOURCE_INVALID(source))
        AMTR_MGR_RETURN(FALSE);

    port_type = SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);

   if ((port_type != SWCTRL_LPORT_NORMAL_PORT)&&(port_type != SWCTRL_LPORT_TRUNK_PORT)
#if (SYS_CPNT_VXLAN == TRUE)
    &&(port_type != SWCTRL_LPORT_VXLAN_PORT)
#endif
    )
       AMTR_MGR_RETURN(FALSE);

    /* if retval is FALSE, reset "StoplearningFlag"
     */
    retval= AMTRDRV_MGR_DeleteAddrByPortAndSource(ifindex, source);
    if (retval)
        AMTR_MGR_Notify_MACTableDeleteByPort(ifindex,  AMTR_MGR_UNKNOWN);

    AMTR_MGR_RETURN(retval);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrByVidAndLifeTime
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete entries by vid + life time from OM
 * INPUT   : UI32_T vid     - vlan id
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddrByVidAndLifeTime(UI32_T vid, AMTR_TYPE_AddressLifeTime_T life_time)
{
    BOOL_T retval=FALSE;
#ifdef SYS_HWCFG_NO_VID_FIELD_IN_ADDR_TBL
    AMTR_MGR_RETURN(FALSE);
#else
    /* BODY
     */
    if (amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    /* Only support logical vid which belongs to dot1q vlan id
     */
    if (!AMTR_TYPE_IS_VALID_VID_OR_REAL_VFI(vid) ||
        IS_LIFETIME_INVALID(life_time) )
        AMTR_MGR_RETURN(FALSE);

    retval=AMTRDRV_MGR_DeleteAddrByVidAndLifeTime(vid,life_time);

    if(!AMTR_TYPE_LOGICAL_VID_IS_DOT1Q_VID(vid))
    {
        /* vfi don't do operations behind this
         */
        AMTR_MGR_RETURN(retval);
    }

    if (retval)
        AMTR_MGR_Notify_MACTableDeleteByVid(vid);
    AMTR_MGR_RETURN(retval);
#endif
} /* end AMTR_MGR_DeleteDynamicAddr_byVID */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrByVidAndSource
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete entries by vid+source from OM
 * INPUT   : UI32_T vid     - vlan id
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddrByVidAndSource(UI32_T vid, AMTR_TYPE_AddressSource_T source)
{
    BOOL_T retval=FALSE;
#ifdef SYS_HWCFG_NO_VID_FIELD_IN_ADDR_TBL
    AMTR_MGR_RETURN(FALSE);
#else
    /* BODY
     */

    if (amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(retval);

    /* Only support logical vid which belongs to dot1q vlan id
     */
    if (IS_SOURCE_INVALID(source)||(!AMTR_TYPE_LOGICAL_VID_IS_DOT1Q_VID(vid)))
        AMTR_MGR_RETURN(retval);

    retval=AMTRDRV_MGR_DeleteAddrByVidAndSource(vid,source);

    if (retval)
        AMTR_MGR_Notify_MACTableDeleteByVid(vid);

    AMTR_MGR_RETURN(retval);
#endif
} /* end AMTR_MGR_DeleteDynamicAddr_byVID */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Internal
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by Life Time+Vid+Port.
 * INPUT   : UI32_T ifindex   - interface index
 *           UI16_T start_vid - the starting vid
 *           UI16_T end_vid   - the end vid
 *           AMTR_MGR_LiftTimeMode_T life_time - other,invalid, delete on timeout, delete on reset, permanent
 *           BOOL_T sync_op   - TRUE: synchronous operation, FALSE:asynchronous operation
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    : 1. For sync_op==TRUE, only supports on life_time as AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT
 * -------------------------------------------------------------------------*/
static BOOL_T AMTR_MGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Internal(UI32_T ifindex, UI16_T start_vid, UI16_T end_vid, AMTR_TYPE_AddressLifeTime_T life_time, BOOL_T sync_op)
{
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;
    SWCTRL_Lport_Type_T     port_type;
    BOOL_T                  retval=FALSE;
    UI32_T                  action_count=0; /*How many entries are deleted in Hisam*/
    /* BODY
     */

    /* Check the current operation mode
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(retval);

    /* Validate the given parameter
     * Only support logical vid which belongs to dot1q vlan id
     */
    if(IS_IFINDEX_INVALID(ifindex)||(!AMTR_TYPE_IS_VALID_VID_OR_REAL_VFI(start_vid)) || (!AMTR_TYPE_IS_VALID_VID_OR_REAL_VFI(end_vid))||IS_LIFETIME_INVALID(life_time))
        AMTR_MGR_RETURN(retval);

    port_type = SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);

   if ((port_type != SWCTRL_LPORT_NORMAL_PORT)&&(port_type != SWCTRL_LPORT_TRUNK_PORT)
#if (SYS_CPNT_VXLAN == TRUE)
    &&(port_type != SWCTRL_LPORT_VXLAN_PORT)
#endif
    )
       AMTR_MGR_RETURN(retval);

    /* For sync_op==TRUE, only supports on life_time=AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT
     */
    if (sync_op==TRUE && life_time!=AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)
    {
        AMTR_MGR_RETURN(retval);
    }

    /* del_by_port+vid+life_time, AMTRDRV won't sync to AMTR_MGR. Core layer have to
     * update hisam table by itself.
     */
    {
        UI32_T i=0;
        for (i = start_vid; i <= end_vid ; i++)
        {
            if ((retval=AMTRDRV_MGR_DeleteAddrByPortAndVidAndLifeTime(ifindex,i, life_time, sync_op)))
            {
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
                BOOL_T do_again;
                do {
#endif
                AMTR_OM_HisamDeleteRecordByLPortAndVidAndLifeTime(ifindex,i, life_time,&action_count);
                AMTR_MGR_UpdatePortSecurityStatus(ifindex);
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
                AMTR_MGR_NOTIFY_EDIT_ADDR_ENTRY_FROM_BUFFER(action_count, TRUE, &do_again);
                } while (do_again);
#endif
            }

            if (retval)
                AMTR_MGR_Notify_MACTableDeleteByVIDnPort(i, ifindex);
        }
    }

    AMTR_MGR_RETURN(retval);
} /*End of AMTR_MGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Internal*/

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrByLifeTimeAndVidRangeAndLPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by Life Time+Vid+Port.
 * INPUT   : UI32_T ifindex - interface index
 *           UI16_T start_vid - the starting vid
 *           UI16_T end_vid   - the end vid
 *           AMTR_MGR_LiftTimeMode_T life_time - other,invalid, delete on timeout, delete on reset, permanent
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddrByLifeTimeAndVidRangeAndLPort(UI32_T ifindex, UI16_T start_vid, UI16_T end_vid, AMTR_TYPE_AddressLifeTime_T life_time)
{
    return AMTR_MGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Internal(ifindex, start_vid, end_vid, life_time, FALSE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by Life Time+Vid+Port
 *           synchronously.
 * INPUT   : UI32_T ifindex   - interface index
 *           UI16_T start_vid - the starting vid
 *           UI16_T end_vid   - the end vid
 *           AMTR_MGR_LiftTimeMode_T life_time - delete on timeout
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    : 1. This function will not return until the delete operation is done
 *           2. Only support life_time as AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync(UI32_T ifindex, UI16_T start_vid, UI16_T end_vid, AMTR_TYPE_AddressLifeTime_T life_time)
{
    return AMTR_MGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Internal(ifindex, start_vid, end_vid, life_time, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrByLifeTimeAndMstIDAndLPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by Life Time+Vid+Port.
 * INPUT   : UI32_T ifindex   - interface index
 *           UI16_T start_vid - the starting vid
 *           UI16_T end_vid   - the end vid
 *           UI32_T vid       - vlan id
 *           AMTR_MGR_LiftTimeMode_T life_time - other,invalid, delete on timeout, delete on reset, permanent
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddrByLifeTimeAndMstIDAndLPort(UI32_T ifindex, UI32_T mstid, AMTR_TYPE_AddressLifeTime_T life_time)
{
    UI32_T  unit,port,trunk_id,vid = 0;
    SWCTRL_Lport_Type_T port_type;
    BOOL_T  retval = FALSE;
    UI32_T  action_count = 0;
    UI32_T  vlan_count = 0;
    static UI16_T * vlan_p = NULL;
    XSTP_MGR_MstpInstanceEntry_T mstp_instance_entry;

    /* BODY
     */


    /* Check the current operation mode
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(retval);

    /* Validate the given parameter
     */
    if(IS_IFINDEX_INVALID(ifindex)||IS_LIFETIME_INVALID(life_time))
        AMTR_MGR_RETURN(retval);

    port_type = SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);

    if ((port_type != SWCTRL_LPORT_NORMAL_PORT)&&(port_type != SWCTRL_LPORT_TRUNK_PORT)
#if (SYS_CPNT_VXLAN == TRUE)
        &&(port_type != SWCTRL_LPORT_VXLAN_PORT)
#endif
        )
        AMTR_MGR_RETURN(retval);

    if(FALSE == XSTP_OM_GetMstpInstanceVlanMapped(mstid,&mstp_instance_entry))
        AMTR_MGR_RETURN(retval);

    vlan_p = (UI16_T *)malloc((SYS_ADPT_MAX_VLAN_ID + 1)*sizeof(UI16_T));
    if (vlan_p == NULL)
        return FALSE;

    /* del_by_port+vid+life_time, AMTRDRV won't sync to AMTR_MGR. Core layer have to
     * update hisam table by itself.
     */
    for (vid = 1; vid <= SYS_DFLT_DOT1QMAXVLANID ; vid++){
        if ((0 < vid) && (vid <= 1023)){
            if(mstp_instance_entry.mstp_instance_vlans_mapped[vid>>3] & (0x01<<(vid%8))){
                vlan_p[vlan_count] = vid;
                vlan_count++;
            }

        }else if ((1023 < vid) && (vid <= 2047)){
            if(mstp_instance_entry.mstp_instance_vlans_mapped2k[(vid>>3)-128] & (0x01<<(vid%8))){
                vlan_p[vlan_count] = vid;
                vlan_count++;
            }
        }else if ((2047 < vid) && (vid <= 3071)){
            if(mstp_instance_entry.mstp_instance_vlans_mapped3k[(vid>>3)-256] & (0x01<<(vid%8))){
                vlan_p[vlan_count] = vid;
                vlan_count++;
            }
        }else if ((3071 < vid) && (vid < 4096)){
            if(mstp_instance_entry.mstp_instance_vlans_mapped4k[(vid>>3)-384] & (0x01<<(vid%8))){
                vlan_p[vlan_count] = vid;
                vlan_count++;

            }
        }
   }

    vlan_p[vlan_count] = 0;
    if(vlan_count)
        if ((retval = AMTRDRV_MGR_DeleteAddrByPortAndVlanBitMapAndLifeTime(ifindex,vlan_p,vlan_count,life_time,&mstp_instance_entry))){
        {
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
            BOOL_T do_again;
            do {
#endif
            AMTR_OM_HisamDeleteRecordByLPortAndVidlistAndLifeTime(ifindex,vlan_p, life_time,&action_count);
            AMTR_MGR_UpdatePortSecurityStatus(ifindex);
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
            AMTR_MGR_NOTIFY_EDIT_ADDR_ENTRY_FROM_BUFFER(action_count, TRUE, &do_again);
            } while (do_again);
#endif
        }
    }

    free(vlan_p);
    AMTR_MGR_RETURN(retval);
}/*End of AMTR_MGR_DeleteAddrByLifeTimeAndMstIDAndLPort*/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAddrBySourceAndVidAndLPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete address entries by specific source and Lport.
 * INPUT   : UI32_T ifindex                 - interface index
 *           UI32_T vid                     - vlan id
 *           AMTR_MGR_SourceMode_T source   - learnt, config, intrenal, self
 * OUTPUT  : None
 * RETURN  : BOOL_T Status  - True : successs, False : failed
 * NOTE    : deletion in both chip and amtr module
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAddrBySourceAndVidAndLPort(UI32_T ifindex, UI32_T vid,AMTR_TYPE_AddressSource_T source)
{
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;
    BOOL_T                  retval=FALSE;
    SWCTRL_Lport_Type_T     port_type;
    UI32_T                  action_count=0; /*How many entries are deleted in Hisam*/

    /* BODY
     */

    /* Check the current operation mode
     */
    if (amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    /* Validate the given parameter
     * Only support logical vid which belongs to dot1q vlan id
     */
    if (IS_IFINDEX_INVALID(ifindex)||IS_SOURCE_INVALID(source)||(!AMTR_TYPE_LOGICAL_VID_IS_DOT1Q_VID(vid)))
        AMTR_MGR_RETURN(FALSE);

    port_type = SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);

    if ((port_type!= SWCTRL_LPORT_NORMAL_PORT)&&(port_type != SWCTRL_LPORT_TRUNK_PORT)
#if (SYS_CPNT_VXLAN == TRUE)
        &&(port_type != SWCTRL_LPORT_VXLAN_PORT)
#endif
        )
        AMTR_MGR_RETURN(FALSE);

    /* del_by_port+vid+life_time, AMTRDRV won't sync to AMTR_MGR. Core layer have to
     * update hisam table by itself.
     */
    if ((retval= AMTRDRV_MGR_DeleteAddrByPortAndVidAndSource(ifindex,vid, source)))
    {
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
        BOOL_T do_again;
        do {
#endif
        AMTR_OM_HisamDeleteRecordByLPortAndVidAndSource(ifindex,vid, source,&action_count);
        AMTR_MGR_UpdatePortSecurityStatus(ifindex);
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
        AMTR_MGR_NOTIFY_EDIT_ADDR_ENTRY_FROM_BUFFER(action_count, TRUE, &do_again);
        } while (do_again);
#endif
    }

    if (retval)
        AMTR_MGR_Notify_MACTableDeleteByVIDnPort(vid, ifindex);

    AMTR_MGR_RETURN(retval);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_DeleteAddrByVidAndLPortExceptCertainAddr
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will delete all entries by port + vid, except
 *            mac address which is matched with mac masks of list.
 * INPUT    : UI32_T ifindex  -- specific logical port
 *            UI32_T vid      -- specific vlan id
 *            UI8_T mac_list_p[AMTR_TYPE_MAX_NBR_OF_ADDR_DELETE_WITH_MASK][SYS_ADPT_MAC_ADDR_LEN]
 *            UI8_T mask_list_p[AMTR_TYPE_MAX_NBR_OF_ADDR_DELETE_WITH_MASK][SYS_ADPT_MAC_ADDR_LEN]
 *            UI32_T number_of_entry_in_list -- there are "number_of_entry_in_list"
 *                                              in mac_mask_list[][SYS_ADPT_MAC_ADDR_LEN]
 * OUTPUT   : None
 * RETURN   : TRUE                             -- success
 *            FALSE                            -- fail
 * NOTE     : 1. caller should use this function like as below.
 *               function()
 *               {
 *                   UI8_T mac_mask_list[][SYS_ADPT_MAC_ADDR_LEN];
 *                   UI8_T mask_list[][SYS_ADPT_MAC_ADDR_LEN];
 *                      mac_list_p[][6]= {{00,01,01,00,aa,cc},  --> number_of_entry_in_list=1
 *                                        {0a,01,01,00,cc,00},  --> number_of_entry_in_list=2
 *                                        {00,01,01,00,00,ff}}; --> number_of_entry_in_list=3
 *                      mask_list_p[][6]= {{FF,FF,00,00,00,00},
 *                                        {FF,FF,FF,FF,00,00},
 *                                        {FF,00,00,00,00,00}};
 *                   AMTR_MGR_DeleteAddrByVidAndLPortExceptCertainAddr(lport,
 *                                                                       vid,
 *                                                                       mask_mac_list,
 *                                                                       mask_list, 3 );
 *               }
 *            2. Don't support trunk port.
 *               So, it only delete entries from specific unit's hisam table.
 *            3. number_of_entry_in_list can't bigger than AMTR_TYPE_MAX_NBR_OF_ADDR_DELETE_WITH_MASK
 *            4. 2004.4.23 water_huang create
 *-------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_DeleteAddrByVidAndLPortExceptCertainAddr(UI32_T ifindex,
                                                         UI32_T vid,
                                                         UI8_T mac_list_ar_p[][SYS_ADPT_MAC_ADDR_LEN],
                                                         UI8_T mask_list_ar_p[][SYS_ADPT_MAC_ADDR_LEN],
                                                         UI32_T number_of_entry_in_list)
{
    UI32_T                  unit;
    UI32_T                  port;
    UI32_T                  trunk_id;

    SWCTRL_Lport_Type_T     port_type;
    BOOL_T                  retval=FALSE;
    UI32_T                  action_count=0; /*How many entries are deleted in Hisam*/
    /* BODY
     */

    /* Check the current operation mode
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(retval);

    /* Validate the given parameter
     */
    if(IS_IFINDEX_INVALID(ifindex)||IS_VID_INVALID(vid))
        AMTR_MGR_RETURN(retval);

    /* don't support trunk port
     */
    port_type = SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);

    if (port_type != SWCTRL_LPORT_NORMAL_PORT)
        AMTR_MGR_RETURN(retval);

    if (number_of_entry_in_list > AMTR_TYPE_MAX_NBR_OF_ADDR_DELETE_WITH_MASK)
        AMTR_MGR_RETURN(retval);

    /* If number_of_entry_in_list isn't zero, it means AMTR have to reference
     * mac_list_ar_p and mask_list_ar_p. So these pointers can't be NULL.
     */
    if ((0 != number_of_entry_in_list)&&((NULL == mac_list_ar_p)||(NULL == mask_list_ar_p)))
        AMTR_MGR_RETURN(retval);

    /* del_by_port+vid_except_certain_addr, AMTRDRV won't sync to AMTR_MGR. Core layer
     * have to update hisam table by itself.
     */
    if ((retval=AMTRDRV_MGR_DeleteAddrByPortAndVidExceptCertainAddr(ifindex,vid,mac_list_ar_p,mask_list_ar_p,number_of_entry_in_list)))
    {
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
        BOOL_T do_again;
        do {
#endif
        AMTR_OM_HisamDeleteRecordByLPortAndVidExceptCertainAddr(ifindex,vid,mac_list_ar_p,mask_list_ar_p,number_of_entry_in_list, &action_count);
        AMTR_MGR_UpdatePortSecurityStatus(ifindex);
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
        AMTR_MGR_NOTIFY_EDIT_ADDR_ENTRY_FROM_BUFFER(action_count, TRUE, &do_again);
        } while (do_again);
#endif
    }
#if 0 /* TODO: wakka: need to implement if someone needs this notification. */
    if (retval)
        AMTR_MGR_Notify_MACTableDeleteByVIDnPortExceptCertainAddr(vid, ifindex,mac_list_ar_p,mask_list_ar_p,number_of_entry_in_list);
#endif

    AMTR_MGR_RETURN(retval);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_DeleteAllAddr
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete all address table entries (both
 *           dynamic and static entries)
 * INPUT   :
 * OUTPUT  : None
 * RETURN  : BOOL_T Status - True : successs, False : failed
 * NOTE    : deletion in both chip and amtr module
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_DeleteAllAddr(void)
{
    BOOL_T retval=FALSE;
    /* BODY
     */

    if (amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(retval);

    if ((retval= AMTRDRV_MGR_DeleteAllAddr()))
    {
        AMTR_OM_HisamDeleteAllRecord();
        AMTR_MGR_UpdatePortSecurityStatus(0);
    }

    AMTR_MGR_RETURN(retval);
} /* end AMTR_MGR_DeleteAllAddr */

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetExactVMAddrEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get exact address entry (vid+mac) from "Hash Table"
 * INPUT   : addr_entry->vid    - VLAN ID     (key)
 *           addr_entry->mac    - MAC address (key)
 * OUTPUT  : addr_entry         - address entry info
 * RETURN  : TRUE  if a valid address exists and retrieved from macList
 *           FALSE if no address can be found
 * NOTE    : using key generated from (vlanID,mac address)
 *------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_GetExactAddrEntry(AMTR_TYPE_AddrEntry_T *addr_entry)
{
    /* BODY
     */

    AMTRDRV_TYPE_Record_T get_entry;

    if (amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if (addr_entry == NULL || (addr_entry->mac == NULL))
        AMTR_MGR_RETURN(FALSE);


    amtrmgr_memcpy(&get_entry.address, addr_entry, sizeof(AMTR_TYPE_AddrEntry_T));

    if (AMTRDRV_OM_GetExactRecord((UI8_T *)&get_entry))
    {
#if (AMTR_MGR_FILTER_OUT_DROP_ENTRY_FOR_UI == TRUE)
        if (!IS_FORWARDING_ENTRY(get_entry.address.action))
        {
            AMTR_MGR_RETURN(FALSE);
        }
#endif

        /* EPRID:ES4626F-SW-FLF-38-01023
         * here should return true even ifindex is 0
         * or Snmp set static mac and get exact
         * mac entry won't work
         * 2010/8/18
         */
#if 0
        if (get_entry.address.ifindex!= 0)
#endif
        {
            amtrmgr_memcpy(addr_entry, &get_entry.address, sizeof(AMTR_TYPE_AddrEntry_T));
            AMTR_MGR_RETURN(TRUE);

        }
    }
    AMTR_MGR_RETURN(FALSE);
} /* end AMTR_MGR_GetExactVMAddrEntry */

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetExactAddrEntryFromChip
 *------------------------------------------------------------------------
 * FUNCTION: This function will get exact address table entry (mac+vid)
 * INPUT   : addr_entry->mac - mac address
 *           addr_entry->vid - vlan id
 * OUTPUT  : addr_entry      - addresss entry info
 * RETURN  : TRUE  if a valid address exists and retrieved from macList
 *           FALSE if no address can be found
 * NOTE    : 1.This API is only support IML_MGR to get exact mac address from chip
 *             We don't support MIB to get under_create entry since l_port =0
 *             will return false
 *           2.This API only support MV key or VM key not IVM key
 *------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_GetExactAddrEntryFromChip(AMTR_TYPE_AddrEntry_T *addr_entry)
{
    if (amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if (addr_entry == 0 || (addr_entry->mac == 0))
        AMTR_MGR_RETURN(FALSE);

    if (AMTRDRV_MGR_GetExactAddrFromChipWithoutISC(addr_entry))
    {
#if (AMTR_MGR_FILTER_OUT_DROP_ENTRY_FOR_UI == TRUE)
        if (!IS_FORWARDING_ENTRY(addr_entry->action))
        {
            AMTR_MGR_RETURN(FALSE);
        }
#endif

        if (addr_entry->ifindex!= 0)
        {
            AMTR_MGR_RETURN(TRUE);
        }
    }
    AMTR_MGR_RETURN(FALSE);
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetNextMVAddrEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get next address entry (mac+vid) from "Hisam Table"
 * INPUT   : addr_entry->mac    - MAC address (primary key)
 *           addr_entry->vid    - VLAN ID     (key)
 *           get_mode           - AMTR_MGR_GET_ALL_ADDRESS
 *                                AMTR_MGR_GET_STATIC_ADDRESS
 *                                AMTR_MGR_GET_DYNAMIC_ADDRESS
 * OUTPUT:   addr_entry         - address entry info
 * RETURN  : BOOL_T Status      - Get Next successful or not
 * NOTE    : 1. if mac[0]~mac[5] == \0 => get the first entry
*            2. get next entry from Hisam Table until get_mode and entry->life_time match
 *------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_GetNextMVAddrEntry(AMTR_TYPE_AddrEntry_T *addr_entry, UI32_T get_mode)
{
    UI8_T               mv_key[AMTR_MVKEY_LEN];
    Port_Info_T         p_info;     /*get port information from swctrl*/
    SWCTRL_Lport_Type_T type;
    UI32_T              temp_unit;
    UI32_T              temp_port;
    UI32_T              temp_trunk_id;

    /* BODY
     */
    if (amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if ((addr_entry == NULL) || (addr_entry->mac == NULL)||(get_mode < AMTR_MGR_GET_ALL_ADDRESS)||
        (get_mode > AMTR_MGR_GET_MIB_ALL_ADDRESS))
        AMTR_MGR_RETURN(FALSE);

    AMTR_OM_SetMVKey(mv_key, addr_entry->vid, addr_entry->mac);

    while(AMTR_OM_HisamGetNextRecord(AMTR_MV_KIDX, mv_key, addr_entry))
    {
#if (AMTR_MGR_FILTER_OUT_DROP_ENTRY_FOR_UI == TRUE)
        if (!IS_FORWARDING_ENTRY(addr_entry->action))
        {
            AMTR_OM_SetMVKey(mv_key, addr_entry->vid, addr_entry->mac);
            continue;
        }
#endif

        /* ifindex ==0 is CPU MAC or Under Create Entry, doesn't any checking.
         */
        if (addr_entry->ifindex!=0)
        {
            /* AMTR have to do some checking befor output this entry to caller.
             * For example:
             * 1. User configure vlan 2 down.
             *    (Module VLAN will produce the "vlan down" event and put in vlan's callback-list.
             *     SYS_CALLBACK will de-queue and notify other CSCs.)
             * 2. User show mac-address-table(dump Hisam Table), the entries on vlan2 are
             *    still exist. (Because SYS_CALLBACK don't notify AMTR yet.)
             * So, AMTR have to check vlan is exist or not before output the entry of vlan 2.
             */
            type = SWCTRL_LogicalPortToUserPort(addr_entry->ifindex, &temp_unit, &temp_port, &temp_trunk_id);

            /* check port type
             */
            if (((type != SWCTRL_LPORT_NORMAL_PORT)&&(type != SWCTRL_LPORT_TRUNK_PORT)
#if (SYS_CPNT_VXLAN == TRUE)
                &&(type != SWCTRL_LPORT_VXLAN_PORT)
#endif
                )||
                /* check vlan is exist or not
                 */
                (!AMTR_MGR_CHECK_VLAN_VALIDITY(addr_entry->vid)))
            {
                AMTR_OM_SetMVKey(mv_key, addr_entry->vid, addr_entry->mac);
                continue;
            }

            /* If and only if source of entry is dynamic,
             * AMTR have to check admin state, link status and xstp state before output
             */
            if ((addr_entry->life_time==AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)&&
#if (SYS_CPNT_VXLAN == TRUE)
                (type != SWCTRL_LPORT_VXLAN_PORT)&&
#endif
                /* get the port info from SWCTRL
                 */
                ((!SWCTRL_GetPortInfo(addr_entry->ifindex, &p_info))||
                /* check port admin status and link status
                 */
                ((p_info.admin_state == VAL_ifAdminStatus_down)||(p_info.link_status == SWCTRL_LINK_DOWN))||
                /* check port spanning tree state
                 */
                (!AMTR_MGR_CHECK_STP_STATE_VALIDITY(addr_entry->vid, addr_entry->ifindex))))
            {
                AMTR_OM_SetMVKey(mv_key, addr_entry->vid, addr_entry->mac);
                continue;
            }
        }

        if ((get_mode != AMTR_MGR_GET_MIB_ALL_ADDRESS)&&
            ((addr_entry->life_time==AMTR_TYPE_ADDRESS_LIFETIME_OTHER)||
              ((addr_entry->ifindex==0)&&(addr_entry->source!=AMTR_TYPE_ADDRESS_SOURCE_SELF))))
            AMTR_OM_SetMVKey(mv_key, addr_entry->vid, addr_entry->mac);
        else if((get_mode == AMTR_MGR_GET_ALL_ADDRESS)||(get_mode == AMTR_MGR_GET_MIB_ALL_ADDRESS))
            AMTR_MGR_RETURN(TRUE);
        else if((get_mode == AMTR_MGR_GET_STATIC_ADDRESS)
            &&(addr_entry->life_time!=AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT))
            AMTR_MGR_RETURN(TRUE);
        else if((get_mode == AMTR_MGR_GET_DYNAMIC_ADDRESS)
            &&(addr_entry->life_time==AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT))
            AMTR_MGR_RETURN(TRUE);
        else
            AMTR_OM_SetMVKey(mv_key, addr_entry->vid, addr_entry->mac);
    }
    AMTR_MGR_RETURN(FALSE);
} /* end AMTR_MGR_GetNextMVAddrEntry */

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetNextVMAddrEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get next address entry (vid+mac) from "Hisam Table"
 * INPUT   : addr_entry->vid    - VLAN ID        (primary key)
 *           addr_entry->mac    - MAC address    (key)
 *           get_mode           - AMTR_MGR_GET_ALL_ADDRESS
 *                                AMTR_MGR_GET_STATIC_ADDRESS
 *                                AMTR_MGR_GET_DYNAMIC_ADDRESS
 * OUTPUT:   addr_entry         - address entry info
 * RETURN  : BOOL_T Status      - Get Next successful or not
 * NOTE    : 1. if vid = 0 => get the first entry
 *           2. using key generated from (vlanID,mac address)
 *           3. get next entry from Hisam Table until get_mode and entry->life_time match
 *------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_GetNextVMAddrEntry(AMTR_TYPE_AddrEntry_T *addr_entry, UI32_T get_mode)
{
    UI8_T               vm_key[AMTR_VMKEY_LEN];
    Port_Info_T         p_info;         /* get port information from swctrl*/
    SWCTRL_Lport_Type_T type;
    UI32_T              temp_unit;
    UI32_T              temp_port;
    UI32_T              temp_trunk_id;

    /* BODY
     */

    if (amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if ((addr_entry == NULL) ||(get_mode<AMTR_MGR_GET_ALL_ADDRESS)||
        (get_mode>AMTR_MGR_GET_MIB_ALL_ADDRESS))
        AMTR_MGR_RETURN(FALSE);

    AMTR_OM_SetVMKey(vm_key, addr_entry->vid, addr_entry->mac);

    while (AMTR_OM_HisamGetNextRecord(AMTR_VM_KIDX, vm_key, addr_entry))
    {
#if (AMTR_MGR_FILTER_OUT_DROP_ENTRY_FOR_UI == TRUE)
        if (!IS_FORWARDING_ENTRY(addr_entry->action))
        {
            AMTR_OM_SetVMKey(vm_key, addr_entry->vid, addr_entry->mac);
            continue;
        }
#endif

        /* ifindex ==0 is CPU MAC or Under Create Entry, doesn't any checking.
         */
        if (addr_entry->ifindex!=0)
        {
            /* AMTR have to do some checking befor output this entry to caller.
             * For example:
             * 1. User configure vlan 2 down.
             *   (Module VLAN will produce the "vlan down" event and put in vlan's callback-list.
             *    SYS_CALLBACK will de-queue and notify other CSCs.)
             * 2. User show mac-address-table(dump Hisam Table), the entries on vlan2 are still exist.
             *    (Because SYS_CALLBACK don't notify AMTR yet.)
             * So, AMTR have to check vlan is exist or not before output the entry of vlan 2.
             */
            type = SWCTRL_LogicalPortToUserPort(addr_entry->ifindex, &temp_unit, &temp_port, &temp_trunk_id);
            /* check port type */
            if (((type != SWCTRL_LPORT_NORMAL_PORT)&&(type != SWCTRL_LPORT_TRUNK_PORT)
#if (SYS_CPNT_VXLAN == TRUE)
                &&(type != SWCTRL_LPORT_VXLAN_PORT)
#endif
                )||
                /*check vlan is exist or not*/
                (!AMTR_MGR_CHECK_VLAN_VALIDITY(addr_entry->vid)))
            {
                AMTR_OM_SetVMKey(vm_key, addr_entry->vid, addr_entry->mac);
                continue;
            }

            /* If and only if source of entry is dynamic,
             * AMTR have to check admin state, link status and xstp state before output
             */
            if ((addr_entry->life_time==AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)&&
#if (SYS_CPNT_VXLAN == TRUE)
                (type != SWCTRL_LPORT_VXLAN_PORT)&&
#endif
                /* get the port info from SWCTRL
                 */
                ((!SWCTRL_GetPortInfo(addr_entry->ifindex, &p_info))||
                /* check port admin status and link status
                 */
                ((p_info.admin_state == VAL_ifAdminStatus_down)||(p_info.link_status == SWCTRL_LINK_DOWN))||
                /* check port spanning tree state
                 */
                (!AMTR_MGR_CHECK_STP_STATE_VALIDITY(addr_entry->vid, addr_entry->ifindex))))
            {
                AMTR_OM_SetVMKey(vm_key, addr_entry->vid, addr_entry->mac);
                continue;
            }
        }

        if ((get_mode != AMTR_MGR_GET_MIB_ALL_ADDRESS)&&
            ((addr_entry->life_time==AMTR_TYPE_ADDRESS_LIFETIME_OTHER)||
              ((addr_entry->ifindex==0)&&(addr_entry->source!=AMTR_TYPE_ADDRESS_SOURCE_SELF))))
            AMTR_OM_SetVMKey(vm_key, addr_entry->vid, addr_entry->mac);
        else if ((get_mode == AMTR_MGR_GET_ALL_ADDRESS)||(get_mode == AMTR_MGR_GET_MIB_ALL_ADDRESS))
            AMTR_MGR_RETURN(TRUE);
        else if((get_mode == AMTR_MGR_GET_STATIC_ADDRESS)
                &&(addr_entry->life_time!=AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT))
            AMTR_MGR_RETURN(TRUE);
        else if((get_mode == AMTR_MGR_GET_DYNAMIC_ADDRESS)
                &&(addr_entry->life_time==AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT))
            AMTR_MGR_RETURN(TRUE);
        else
            AMTR_OM_SetVMKey(vm_key, addr_entry->vid, addr_entry->mac);
      }
    AMTR_MGR_RETURN(FALSE);
} /* end AMTR_MGR_GetNextVMAddrEntry */

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetExactIfIndexAddrEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get exact address entry (ifindex+vid+mac) from "Hash Table"
 * INPUT   : addr_entry->ifindex - interface index   (key)
 *           addr_entry->vid     - vlan id           (key)
 *           addr_entry->mac     - mac address       (key)
 * OUTPUT  : addr_entry          - addresss entry info
 * RETURN  : TRUE  if a valid address exists and retrieved from Hash Table
 *           FALSE if no address can be found
 * NOTE    : 1. ifindex is a normal port or trunk port
 *           2. Get exact address entry by vid+mac.
 *              Before return, AMTR_MGR has to check ifindex by itself.
 *------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_GetExactIfIndexAddrEntry(AMTR_TYPE_AddrEntry_T *addr_entry)
{
    UI32_T              temp_ifindex;
    SWCTRL_Lport_Type_T type;
    UI32_T              temp_unit;
    UI32_T              temp_port;
    UI32_T              temp_trunk_id;
    AMTRDRV_TYPE_Record_T get_entry;
    /* BODY
     */

    if (amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if (addr_entry == NULL || (addr_entry->mac == 0))
        AMTR_MGR_RETURN(FALSE);

    type = SWCTRL_LogicalPortToUserPort(addr_entry->ifindex, &temp_unit, &temp_port, &temp_trunk_id);
    /* ifindex can't be trunk member or unknow port
     */
    if ((type != SWCTRL_LPORT_NORMAL_PORT)&&(type != SWCTRL_LPORT_TRUNK_PORT)
#if (SYS_CPNT_VXLAN == TRUE)
        &&(type != SWCTRL_LPORT_VXLAN_PORT)
#endif
        )
        AMTR_MGR_RETURN(FALSE);

    temp_ifindex = addr_entry->ifindex;

    amtrmgr_memcpy(&get_entry.address, addr_entry, sizeof(AMTR_TYPE_AddrEntry_T));

    if (AMTRDRV_OM_GetExactRecord((UI8_T *)&get_entry))
    {
#if (AMTR_MGR_FILTER_OUT_DROP_ENTRY_FOR_UI == TRUE)
        if (!IS_FORWARDING_ENTRY(get_entry.address.action))
        {
            AMTR_MGR_RETURN(FALSE);
        }
#endif

        if ((get_entry.address.ifindex!= 0)&&(temp_ifindex == get_entry.address.ifindex))
        {
            amtrmgr_memcpy(addr_entry, &get_entry.address, sizeof(AMTR_TYPE_AddrEntry_T));
            AMTR_MGR_RETURN(TRUE);
        }
    }
    AMTR_MGR_RETURN(FALSE);
} /* end AMTR_MGR_GetExactIfIndexAddrEntry */


/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetNextIfIndexAddrEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get next address table entry (ifindex+vid+mac)
 * INPUT   : addr_entry->l_port - interface index   (primary key)
 *           addr_entry->vid    - vlan id           (key)
 *           addr_entry->mac    - MAC address       (key)
 *           get_mode           - AMTR_MGR_GET_ALL_ADDRESS
 *                                AMTR_MGR_GET_STATIC_ADDRESS
 *                                AMTR_MGR_GET_DYNAMIC_ADDRESS
 * OUTPUT:   addr_entry         - address entry info
 * RETURN  : BOOL_T Status      - Get Next successful or not
 * NOTE    : 1. l_port == 0 => get the first entry
 *           2. l_port is a physical port or trunk port
 *------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_GetNextIfIndexAddrEntry(AMTR_TYPE_AddrEntry_T *addr_entry, UI32_T get_mode)
{
    UI8_T               ivm_key[AMTR_IVMKEY_LEN];
    Port_Info_T         p_info;     /*get port information from swctrl*/
    SWCTRL_Lport_Type_T type;
    UI32_T              temp_unit;
    UI32_T              temp_port;
    UI32_T              temp_trunk_id;
    /* BODY
     */

    if (amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if ((addr_entry == NULL) || (addr_entry->mac == NULL)||(get_mode < AMTR_MGR_GET_ALL_ADDRESS)||
        (get_mode > AMTR_MGR_GET_MIB_ALL_ADDRESS))
        AMTR_MGR_RETURN(FALSE);

    AMTR_OM_SetIVMKey(ivm_key, addr_entry->vid, addr_entry->mac,addr_entry->ifindex);

    while (AMTR_OM_HisamGetNextRecord(AMTR_IVM_KIDX, ivm_key, addr_entry))
    {
#if (AMTR_MGR_FILTER_OUT_DROP_ENTRY_FOR_UI == TRUE)
        if (!IS_FORWARDING_ENTRY(addr_entry->action))
        {
            AMTR_OM_SetIVMKey(ivm_key, addr_entry->vid, addr_entry->mac,addr_entry->ifindex);
            continue;
        }
#endif

        /* ifindex ==0 is CPU MAC or Under Create Entry, doesn't any checking.
         */
        if (addr_entry->ifindex!=0)
        {
            /* AMTR have to do some checking befor output this entry to caller.
             * For example:
             * 1. User configure vlan 2 down.
             *   (Module VLAN will produce the "vlan down" event and put in vlan's callback-list.
             *    SYS_CALLBACK will de-queue and notify other CSCs.)
             * 2. User show mac-address-table(dump Hisam Table), the entries on vlan2 are still exist.
             *   (Because SYS_CALLBACK don't notify AMTR yet.)
             * So, AMTR have to check vlan is exist or not before output the entry of vlan 2.
             */
            type = SWCTRL_LogicalPortToUserPort(addr_entry->ifindex, &temp_unit, &temp_port, &temp_trunk_id);
            /* check port type
             */
            if (((type != SWCTRL_LPORT_NORMAL_PORT)&&(type != SWCTRL_LPORT_TRUNK_PORT)
#if (SYS_CPNT_VXLAN == TRUE)
                &&(type != SWCTRL_LPORT_VXLAN_PORT)
#endif
                )||
                /* check vlan is exist or not
                 */
                (!AMTR_MGR_CHECK_VLAN_VALIDITY(addr_entry->vid)))
            {
                AMTR_OM_SetIVMKey(ivm_key, addr_entry->vid, addr_entry->mac,addr_entry->ifindex);
                continue;
            }

            /* If and only if source of entry is dynamic,
             * AMTR have to check admin state, link status and xstp state before output
             */
            if ((addr_entry->life_time==AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)&&
#if (SYS_CPNT_VXLAN == TRUE)
                (type != SWCTRL_LPORT_VXLAN_PORT)&&
#endif
                /* get the port info from SWCTRL
                 */
                ((!SWCTRL_GetPortInfo(addr_entry->ifindex, &p_info))||
                /* check port admin status and link status
                 */
                ((p_info.admin_state == VAL_ifAdminStatus_down)||(p_info.link_status == SWCTRL_LINK_DOWN))||
                /* check port spanning tree state
                 */
                (!AMTR_MGR_CHECK_STP_STATE_VALIDITY(addr_entry->vid, addr_entry->ifindex))))
            {
                AMTR_OM_SetIVMKey(ivm_key, addr_entry->vid, addr_entry->mac,addr_entry->ifindex);
                continue;
            }
        }

        if ((get_mode != AMTR_MGR_GET_MIB_ALL_ADDRESS)&&
            ((addr_entry->life_time==AMTR_TYPE_ADDRESS_LIFETIME_OTHER)||
              ((addr_entry->ifindex==0)&&(addr_entry->source!=AMTR_TYPE_ADDRESS_SOURCE_SELF))))
            AMTR_OM_SetIVMKey(ivm_key, addr_entry->vid, addr_entry->mac,addr_entry->ifindex);
        else if ((get_mode == AMTR_MGR_GET_ALL_ADDRESS)||(get_mode == AMTR_MGR_GET_MIB_ALL_ADDRESS))
            AMTR_MGR_RETURN(TRUE);
        else if((get_mode == AMTR_MGR_GET_STATIC_ADDRESS)
                &&(addr_entry->life_time!=AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT))
            AMTR_MGR_RETURN(TRUE);
        else if((get_mode == AMTR_MGR_GET_DYNAMIC_ADDRESS)
                &&(addr_entry->life_time==AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT))
            AMTR_MGR_RETURN(TRUE);
        else
            AMTR_OM_SetIVMKey(ivm_key, addr_entry->vid, addr_entry->mac,addr_entry->ifindex);
      }
    AMTR_MGR_RETURN(FALSE);
} /* end AMTR_MGR_GetNextIfIndexAddrEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_GetNextVMAddrEntryByLPortNMaskMatch
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get next entry on specific lport
 *            until entry.mac matched mac with mask.
 * INPUT    : addr_entry->mac[SYS_ADPT_MAC_ADDR_LEN]  -- mac address
 *            addr_entry->vid                         -- vlan ID
 *            addr_entry->ifindex                     -- specific logical port
 *            UI8_T mask_mac[SYS_ADPT_MAC_ADDR_LEN]   -- ex. {AA,BB,11,10,20,03}
 *            UI8_T mask[SYS_ADPT_MAC_ADDR_LEN]       -- ex. {FF,FF,FF,FF,00,00}
 *            get_mode                                -- AMTR_MGR_GET_ALL_ADDRESS
 *                                                       AMTR_MGR_GET_STATIC_ADDRESS
 *                                                       AMTR_MGR_GET_DYNAMIC_ADDRESS
 * OUTPUT   : AMTR_MGR_AddrEntry_T *addr_entry
 * RETURN   : TRUE                             -- success
 *            FALSE                            -- fail
 * NOTE     : 1. if addr_entry->mac[]== NULL_MAC and addr_entry->vid == 0,
 *               AMTR will get the first entry on this l_port.
 *            2. addr_entry->l_port can't be zero.
 *            3. If(memcpr(mask_mac & mask, addr_entry->mac & mask, 6)==0),
 *               return TRUE.
 *            4. This function is created for Voice Vlan.
 *            5. This function won't check input(vid, mac,l_port), because
 *               caller may set zero to get first entry.
 *            6. The input "addr_entry->l_port" can't be zero.
 *            7. This function doesn't support trunk port.
 *            8. 2006.4.23 water_huang create
 *            9. This API doesn't filter out entries that have drop attribute
 *-------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetNextVMAddrEntryByLPortNMaskMatch(AMTR_TYPE_AddrEntry_T *addr_entry,
                                                    UI8_T mask_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                    UI8_T mask[SYS_ADPT_MAC_ADDR_LEN],
                                                    UI32_T get_mode)
{
    UI8_T               ivm_key[AMTR_IVMKEY_LEN];
    Port_Info_T         p_info;     /*get port information from swctrl*/
    SWCTRL_Lport_Type_T type;
    UI32_T              temp_unit;
    UI32_T              temp_port;
    UI32_T              temp_trunk_id;
    UI32_T              specified_ifindex;
    UI32_T              mac_index;

    /* BODY
     */

    if (amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if ((addr_entry == NULL) || (addr_entry->mac == NULL)||(get_mode < AMTR_MGR_GET_ALL_ADDRESS)||
        (get_mode > AMTR_MGR_GET_MIB_ALL_ADDRESS))
        AMTR_MGR_RETURN(FALSE);

    if (mask_mac == NULL || mask == NULL)
        AMTR_MGR_RETURN(FALSE);

    /* This function will search on specific port.
     * So, input l_port can't be zero.
     */
    if (addr_entry->ifindex == 0)
        AMTR_MGR_RETURN(FALSE);

    type = SWCTRL_LogicalPortToUserPort(addr_entry->ifindex, &temp_unit, &temp_port, &temp_trunk_id);

    if ((type != SWCTRL_LPORT_NORMAL_PORT)&&(type != SWCTRL_LPORT_TRUNK_PORT)
#if (SYS_CPNT_VXLAN == TRUE)
        &&(type != SWCTRL_LPORT_VXLAN_PORT)
#endif
        )
        AMTR_MGR_RETURN(FALSE);

    /* get the port info from SWCTRL
     */
    if (!SWCTRL_GetPortInfo(addr_entry->ifindex, &p_info))
        AMTR_MGR_RETURN(FALSE);

    AMTR_OM_SetIVMKey(ivm_key, addr_entry->vid, addr_entry->mac,addr_entry->ifindex);

    specified_ifindex = addr_entry->ifindex;

    while (AMTR_OM_HisamGetNextRecord(AMTR_IVM_KIDX, ivm_key, addr_entry))
    {
#if (AMTR_MGR_FILTER_OUT_DROP_ENTRY_FOR_UI == TRUE)
        /* ADD_MGR needs to manage all drop entries,
         * so never filter out drop entreis here.
         */
#endif

        if (addr_entry->ifindex != specified_ifindex)
            break;

        /* check mask match
         */
        for (mac_index = 0; mac_index < SYS_ADPT_MAC_ADDR_LEN; mac_index++)
        {
            if ((addr_entry->mac[mac_index] & mask[mac_index]) !=
                (mask_mac[mac_index] & mask[mac_index]))
            {
                break;
            }
        }
        if (mac_index < SYS_ADPT_MAC_ADDR_LEN)
        {
            AMTR_OM_SetIVMKey(ivm_key, addr_entry->vid, addr_entry->mac,addr_entry->ifindex);
            continue;
        }

        /* AMTR have to do some checking befor output this entry to caller.
         * For example:
         * 1. User configure vlan 2 down.
         *   (Module VLAN will produce the "vlan down" event and put in vlan's callback-list.
         *    SYS_CALLBACK will de-queue and notify other CSCs.)
         * 2. User show mac-address-table(dump Hisam Table), the entries on vlan2 are still exist.
         *   (Because SYS_CALLBACK don't notify AMTR yet.)
         * So, AMTR have to check vlan is exist or not before output the entry of vlan 2.
         */
        /* check vlan is exist or not
         */
        if (!AMTR_MGR_CHECK_VLAN_VALIDITY(addr_entry->vid))
        {
            AMTR_OM_SetIVMKey(ivm_key, addr_entry->vid, addr_entry->mac,addr_entry->ifindex);
            continue;
        }

        /* If and only if source of entry is dynamic,
         * AMTR have to check admin state, link status and xstp state before output
         */
        if ((addr_entry->life_time == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)&&
#if (SYS_CPNT_VXLAN == TRUE)
            (type != SWCTRL_LPORT_VXLAN_PORT)&&
#endif
            /* check port admin status and link status
             */
            (((p_info.admin_state == VAL_ifAdminStatus_down)||(p_info.link_status == SWCTRL_LINK_DOWN))||
            /* check port spanning tree state
             */
            (!AMTR_MGR_CHECK_STP_STATE_VALIDITY(addr_entry->vid, addr_entry->ifindex))))
        {
            AMTR_OM_SetIVMKey(ivm_key, addr_entry->vid, addr_entry->mac,addr_entry->ifindex);
            continue;
        }

        if ((get_mode != AMTR_MGR_GET_MIB_ALL_ADDRESS)&&
            ((addr_entry->life_time==AMTR_TYPE_ADDRESS_LIFETIME_OTHER)||
              ((addr_entry->ifindex==0)&&(addr_entry->source!=AMTR_TYPE_ADDRESS_SOURCE_SELF))))
            AMTR_OM_SetIVMKey(ivm_key, addr_entry->vid, addr_entry->mac,addr_entry->ifindex);
        else if ((get_mode == AMTR_MGR_GET_ALL_ADDRESS)||(get_mode == AMTR_MGR_GET_MIB_ALL_ADDRESS))
            AMTR_MGR_RETURN(TRUE);
        else if((get_mode == AMTR_MGR_GET_STATIC_ADDRESS)
                &&(addr_entry->life_time!=AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT))
            AMTR_MGR_RETURN(TRUE);
        else if((get_mode == AMTR_MGR_GET_DYNAMIC_ADDRESS)
                &&(addr_entry->life_time==AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT))
            AMTR_MGR_RETURN(TRUE);
        else
            AMTR_OM_SetIVMKey(ivm_key, addr_entry->vid, addr_entry->mac,addr_entry->ifindex);
    }

    AMTR_MGR_RETURN(FALSE);
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetNextRunningStaticAddrEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *           static address can be retrieved successfully. Otherwise,
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL is returned.
 * INPUT   : addr_entry->mac    - MAC address (primary key)
 *           addr_entry->vid    - VLAN ID     (key)
 * OUTPUT:   addr_entry         - address entry info
 * RETURN  : BOOL_T Status      - Get Next successful or not
 * NOTE    : 1. if mac[0]~mac[5] == \0 => get the first entry
 *           2. the function shall be only invoked by cli
 *------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AMTR_MGR_GetNextRunningStaticAddrEntry(AMTR_TYPE_AddrEntry_T *addr_entry)
{
    UI32_T  retval=FALSE;
    UI8_T       mv_key[AMTR_MVKEY_LEN];
    /* BODY
     */

    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(SYS_TYPE_GET_RUNNING_CFG_FAIL);

#ifdef SYS_HWCFG_NO_VID_FIELD_IN_ADDR_TBL
    addr_entry->vid = 1;
#endif

    AMTR_OM_SetMVKey(mv_key, addr_entry->vid, addr_entry->mac);

    while(AMTR_OM_HisamGetNextRecord(AMTR_MV_KIDX, mv_key,addr_entry)){
#if (AMTR_MGR_FILTER_OUT_DROP_ENTRY_FOR_UI == TRUE)
        if (!IS_FORWARDING_ENTRY(addr_entry->action))
        {
            AMTR_OM_SetMVKey(mv_key, addr_entry->vid, addr_entry->mac);
            continue;
        }
#endif

        if (addr_entry->life_time == AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT)
        {
            retval = TRUE;
            break;
        }
        else
        {
            AMTR_OM_SetMVKey(mv_key, addr_entry->vid, addr_entry->mac);
        }
    }
    return (retval == TRUE)? SYS_TYPE_GET_RUNNING_CFG_SUCCESS: SYS_TYPE_GET_RUNNING_CFG_FAIL;
} /* end AMTR_MGR_GetNextRunningStaticAddrEntry */

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetNumberOfDynamicAddrEntryByPort
 *------------------------------------------------------------------------
 * FUNCTION: Gets the total dynamic address entries on the port
 * INPUT   : UI32_T l_port
 * OUTPUT  : UI32_T *number_of_entry - pointer to buffer to store number
 *                                     of dynamic address table entries
 * RETURN  : None
 * NOTE    : Because this API can't return FALSE, don't need check l_port is legal or not
 *------------------------------------------------------------------------*/
void AMTR_MGR_GetNumberOfStaticAddrEntryByPort(UI32_T l_port, UI32_T *number_of_entry_p)
{
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN();

    *number_of_entry_p = AMTRDRV_OM_GetStaticCounterByPort(l_port);
}

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_SetInterventionEntry
 *------------------------------------------------------------------------------
 * Purpose  : This routine will add a CPU MAC to the chip.  Packets with this
 *            MAC in the specified VLAN will trap to CPU.
 * INPUT    : UI32_T vid    - vlan ID
 *            UI8_T *mac    - MAC address
 * OUTPUT   : None
 * RETURN   : AMTR_TYPE_Ret_T     Success, or cause of fail.
 * NOTE     : This API doesn't forbid that CPU has only one MAC.  It will only
 *            depend on spec.
 *------------------------------------------------------------------------------*/
AMTR_TYPE_Ret_T  AMTR_MGR_SetInterventionEntry(UI32_T vid, UI8_T *mac)
{
    AMTRDRV_TYPE_Record_T addr_entry;
    AMTR_TYPE_Ret_T retval;

    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(AMTR_TYPE_RET_ERROR_UNKNOWN);

    if (IS_MAC_INVALID(mac)||IS_MULTICAST_MAC(mac))
        AMTR_MGR_RETURN(AMTR_TYPE_RET_ERROR_UNKNOWN);

#ifndef SYS_HWCFG_NO_VID_FIELD_IN_ADDR_TBL
    if ((vid == 0)||(!VLAN_OM_IsVlanExisted(vid)))
        AMTR_MGR_RETURN(AMTR_TYPE_RET_ERROR_UNKNOWN);
#endif

    amtrmgr_memcpy(addr_entry.address.mac, mac, AMTR_TYPE_MAC_LEN);
    addr_entry.address.vid = vid;
    if ((AMTRDRV_OM_GetExactRecord((UI8_T *)&addr_entry) == TRUE)&&(addr_entry.address.source==AMTR_TYPE_ADDRESS_SOURCE_SELF))
        AMTR_MGR_RETURN(AMTR_TYPE_RET_SUCCESS);
#if (SYS_CPNT_AMTR_CPU_INTERVENTION_METHOD == SYS_CPNT_AMTR_CPU_INTERVENTION_CPU_JOIN_VLAN)
    SWCTRL_PMGR_AddHostToVlan(vid);
#endif

    addr_entry.address.action=AMTR_TYPE_ADDRESS_ACTION_FORWARDING;
    addr_entry.address.source=AMTR_TYPE_ADDRESS_SOURCE_SELF;
    /* User copy running to start-up conig file, COU MAC doesn't be saved.
     * After reboot, other CSCs will set CPU MAC again.
     * In HW learning, AMTR don't save cpu mac in Hisam Table.
     * So, get running static() can't get CPU MAC.
     */
    addr_entry.address.life_time=AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET;
    /* cpu mac will be record in OM without counter.
     * So, it need "ifindex"
     */
    addr_entry.address.ifindex=0;
    addr_entry.address.priority=AMTR_TYPE_ADDRESS_WITHOUT_PRIORITY;
    retval = AMTRDRV_MGR_SetInterventionEntry(1, &addr_entry.address);

    AMTR_MGR_RETURN(retval);
}/* End of AMTR_MGR_SetInterventionEntry () */

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_DeleteInterventionEntry
 *------------------------------------------------------------------------------
 * Purpose  : This function will delete a intervention mac address from
 *            address table
 * INPUT    : UI32_T vid    - vlan ID
 *            UI8_T *mac    - MAC address
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTR_MGR_DeleteInterventionEntry(UI32_T vid, UI8_T *mac)
{
    AMTRDRV_TYPE_Record_T addr_entry;
    BOOL_T              retval;

    /* BODY
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if (mac == NULL||IS_MAC_INVALID(mac)||IS_MULTICAST_MAC(mac))
        AMTR_MGR_RETURN(FALSE);

#ifndef SYS_HWCFG_NO_VID_FIELD_IN_ADDR_TBL
    if ((vid == 0)||(!VLAN_OM_IsVlanExisted(vid)))
        AMTR_MGR_RETURN(FALSE);
#endif

    amtrmgr_memcpy(addr_entry.address.mac, mac, AMTR_TYPE_MAC_LEN);
    addr_entry.address.vid = vid;
    if (AMTRDRV_OM_GetExactRecord((UI8_T *)&addr_entry) == FALSE)
    {
        /* can't get == not exist == already be deleted, return TRUE
         */
        AMTR_MGR_RETURN(TRUE);
    }

#if (SYS_CPNT_AMTR_CPU_INTERVENTION_METHOD == SYS_CPNT_AMTR_CPU_INTERVENTION_CPU_JOIN_VLAN)
    retval = SWCTRL_PMGR_DeleteHostFromVlan(vid);
#endif

    if (addr_entry.address.source!=AMTR_TYPE_ADDRESS_SOURCE_SELF)
        AMTR_MGR_RETURN(FALSE);

    retval =  AMTRDRV_MGR_DeleteInterventionEntry(1, &addr_entry.address);
    AMTR_MGR_RETURN(retval);
} /* End of AMTR_MGR_DeleteInterventionEntry () */

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_CreateMulticastAddr
 *------------------------------------------------------------------------------
 * Purpose  : This function will create a multicast address table entry
 * INPUT    : UI32_T vid    - vlan ID
 *            UI8_T *mac    - multicast mac address
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTR_MGR_CreateMulticastAddrTblEntry(UI32_T vid, UI8_T *mac)
{
    BOOL_T retval=FALSE;
    /* BODY
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if (mac == NULL||(!IS_MULTICAST_MAC(mac))||IS_MAC_INVALID(mac))
        AMTR_MGR_RETURN(FALSE);

#ifndef SYS_HWCFG_NO_VID_FIELD_IN_ADDR_TBL
    if (vid == 0 ||!VLAN_OM_IsVlanExisted(vid))
        AMTR_MGR_RETURN(FALSE);
#endif
    retval = AMTRDRV_MGR_CreateMulticastAddr(vid, mac);
    AMTR_MGR_RETURN(retval);
}

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_DestroyMulticastAddr
 *------------------------------------------------------------------------------
 * Purpose  : This function will destroy a multicast address table entry
 * INPUT    : UI32_T vid    - vlan ID
 *            UI8_T *mac    - multicast mac address
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTR_MGR_DestroyMulticastAddrTblEntry(UI32_T vid, UI8_T* mac)
{
    BOOL_T retval=FALSE;
    /* BODY
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if (mac == NULL||(!IS_MULTICAST_MAC(mac))||IS_MAC_INVALID(mac))
        AMTR_MGR_RETURN(FALSE);

#ifndef SYS_HWCFG_NO_VID_FIELD_IN_ADDR_TBL
    if (vid == 0 ||!VLAN_OM_IsVlanExisted(vid))
        AMTR_MGR_RETURN(FALSE);
#endif
    retval = AMTRDRV_MGR_DestroyMulticastAddr (vid, mac);
    AMTR_MGR_RETURN(retval);
}

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_SetMulticastPortMember
 *------------------------------------------------------------------------------
 * Purpose  : This function sets the port member(s) of a given multicast address
 * INPUT    : UI32_T vid
 *            UI8_T *mac                                                                                - multicast MAC address
 *            UI8_T ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]   - the member ports of the MAC
 *            UI8_T tagged[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] - tagged/untagged member
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTR_MGR_SetMulticastPortMember(UI32_T vid,
                                        UI8_T *mac,
                                        UI8_T ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
                                        UI8_T tagged[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST])
{
    UI32_T  port_type;
    UI32_T  ifindex;
    UI32_T  unit;
    UI32_T  u_port;
    UI32_T  trunk_id;
    UI8_T   byte_offset;
    UI8_T   bit_mask;

    BOOL_T retval=FALSE;


    /* Only master unit is allowed to perform this operation
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    /* Value validation for MAC address
     */
    if (mac == NULL||(!IS_MULTICAST_MAC(mac))||IS_MAC_INVALID(mac))
        AMTR_MGR_RETURN(FALSE);

#ifndef SYS_HWCFG_NO_VID_FIELD_IN_ADDR_TBL
    if ((vid == 0)||(!VLAN_OM_IsVlanExisted(vid)))
        AMTR_MGR_RETURN(FALSE);
#endif

    for(ifindex=1; ifindex <= (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT); ifindex++)
    {

        byte_offset = (ifindex - 1) / 8;            /* ifindex 1~8: byte_offset=0; */
        bit_mask    = 0x80 >> ((ifindex - 1) % 8);  /* ifindex 1 = 0x1000 0000; ifindex 2= 0x0100 0000...*/

        /* port bit is off, the tagged bit should not be on!
         */
        if (!(bit_mask & ports[byte_offset]))
        {
            tagged[byte_offset] ^= bit_mask;
        }
        else/*port bit is on.*/
        {
            /* Convert L_port to U_port
             */
            port_type   = SWCTRL_LogicalPortToUserPort(ifindex, &unit, &u_port, &trunk_id);
            if ((port_type !=  SWCTRL_LPORT_NORMAL_PORT) && (port_type != SWCTRL_LPORT_TRUNK_PORT_MEMBER))
            {
                /* port_type is invalid. port bit and tagged bit should be off.
                 */
                ports[byte_offset] ^= bit_mask;
                tagged[byte_offset] ^= bit_mask;
            }
        }
    }

    /* don't support trunk port. Mask all bits of trunk port !!
     */
    for(ifindex=(SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)+1;
         ifindex<=(SYS_ADPT_TOTAL_NBR_OF_LPORT); ifindex++)
    {
        byte_offset = (ifindex - 1) / 8;
        bit_mask    = 0x80 >> ((ifindex - 1) % 8);
        ports[byte_offset] ^= bit_mask;
        tagged[byte_offset] ^= bit_mask;
    }

    retval=AMTRDRV_MGR_SetMulticastPortMember(vid, mac, ports, tagged);
    AMTR_MGR_RETURN(retval);
}


/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_GetAgingStatus
 *------------------------------------------------------------------------------
 * Purpose  : This function Get the Address Ageing Status
 * INPUT    : None
 * OUTPUT   : UI32_T *status -
 *               VAL_amtrMacAddrAgingStatus_enabled
 *               VAL_amtrMacAddrAgingStatus_disabled
 * RETURN   : BOOL_T         - True : successs, False : failed
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTR_MGR_GetAgingStatus(UI32_T *status)
{
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if(status == NULL)
        AMTR_MGR_RETURN(FALSE);

    *status = amtr_mgr_config_aging_status;
    AMTR_MGR_RETURN(TRUE);
}

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_GetRunningAgingStatus
 *------------------------------------------------------------------------------
 * Purpose  : This function Get the Address Ageing Status
 * INPUT    : None
 * OUTPUT   : UI32_T *status -  VAL_amtrMacAddrAgingStatus_enabled
 *                              VAL_amtrMacAddrAgingStatus_disabled
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T -
 *            1. SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            2. SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            3. SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     :
 *-----------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  AMTR_MGR_GetRunningAgingStatus(UI32_T *status)
{
    /* BODY
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
            AMTR_MGR_RETURN(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (status == NULL)
        AMTR_MGR_RETURN(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *status = amtr_mgr_config_aging_status;

    if (*status == SYS_DFLT_L2_DYNAMIC_ADDR_AGING_STATUS)
        AMTR_MGR_RETURN(SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);

    AMTR_MGR_RETURN(SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
}

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_SetAgingStatus
 *------------------------------------------------------------------------------
 * Purpose  : This function Set the Address Ageing Status(system-wise)
 * INPUT    : status        - VAL_amtrMacAddrAgingStatus_enabled
 *                            VAL_amtrMacAddrAgingStatus_disabled
 * OUTPUT   : None
 *
 * RETURN   : BOOL_T         - True : successs, False : failed
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTR_MGR_SetAgingStatus(UI32_T status)
{
    BOOL_T retval=FALSE;


    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if (status == VAL_amtrMacAddrAgingStatus_disabled)
        retval = AMTRDRV_MGR_SetAgingTime(0);
    else if (status == VAL_amtrMacAddrAgingStatus_enabled)
        retval = AMTRDRV_MGR_SetAgingTime(amtr_mgr_config_aging_time);
    else
        AMTR_MGR_RETURN(FALSE);

    if (retval)
        amtr_mgr_config_aging_status = status;
    AMTR_MGR_RETURN(retval);
}
/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_SetMACLearningStatusOnPort
 *------------------------------------------------------------------------------
 * Purpose  : This function set port learning status
 * INPUT    : status        - TRUE is enable
 *                            FALSE  is  disable
 * OUTPUT   : None
 *
 * RETURN   : BOOL_T         - True : successs, False : failed
 * NOTE     : None

 *-----------------------------------------------------------------------------*/
#define PORT_LEARN_SET    (1<<0)
#define PORT_LEARN_CPU  (1<<1)
#define PORT_LEARN_FWD  (1<<2)
#define PORT_LEARN_ARL  (1<<3)
#define  PORT_CONTROL_SET (1<<4)
#define PORT_LEARN_STATICMOVETOCPU (1<<5)

BOOL_T  AMTR_MGR_SetMACLearningStatusOnPort(UI32_T unit,UI32_T port,UI32_T status)
{
    BOOL_T retval = FALSE;
    UI32_T  mode = 0;
    UI32_T  ifindex;
    UI32_T  trunk_ifindex;
    BOOL_T  is_static;
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    SWCTRL_UserPortToIfindex(unit,port,&ifindex);

    if (status == AMTR_MGR_ENABLE_MACLEARNING_ONPORT ){

          mode = PORT_LEARN_FWD|PORT_LEARN_CPU|PORT_LEARN_SET|PORT_CONTROL_SET|PORT_LEARN_STATICMOVETOCPU;

#if 0 /* obsolete. use SWCTRL_SetPortLearningStatus instead */
          retval = AMTRDRV_MGR_SetInterfaceConfig(unit,port,mode);
#endif

          amtr_port_info[ifindex-1].is_learn = TRUE;
          if(TRUE == SWCTRL_IsTrunkMember(ifindex,&trunk_ifindex,&is_static))
              amtr_port_info[trunk_ifindex-1].is_learn = TRUE;

    }else if (status == AMTR_MGR_DISABLE_MACLEARNING_ONPORT){


          mode = PORT_LEARN_FWD|PORT_LEARN_SET|PORT_CONTROL_SET;
#if 0 /* obsolete. use SWCTRL_SetPortLearningStatus instead */
          retval = AMTRDRV_MGR_SetInterfaceConfig(unit,port,mode);
#endif

          AMTR_MGR_DeleteAddrByLPort(ifindex);
          /*update interface cofig*/
          amtr_port_info[ifindex-1].is_learn = FALSE;
          /*if its a trunk member , update trunk interface*/
          if(TRUE == SWCTRL_IsTrunkMember(ifindex,&trunk_ifindex,&is_static))
              amtr_port_info[trunk_ifindex-1].is_learn = FALSE;

    }else
        AMTR_MGR_RETURN(FALSE);

    AMTR_MGR_RETURN(retval);
}
/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_GetMACLearningStatusOnPort
 *------------------------------------------------------------------------------
 * Purpose  : This function get port mac learning status
 * INPUT    : status        - enable is TRUE
 *                            Disable is FALSE
 * OUTPUT   : None
 *
 * RETURN   : BOOL_T         - True : successs, False : failed
 * NOTE     : None

 *-----------------------------------------------------------------------------*/

BOOL_T  AMTR_MGR_GetMACLearningStatusOnPort(UI32_T ifindex,UI32_T * status)
{
    BOOL_T retval=FALSE;
    SWCTRL_Lport_Type_T  port_type;
    UI32_T  unit;
    UI32_T  port;
    UI32_T  trunk_id;
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    port_type = SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);

    if ((port_type != SWCTRL_LPORT_NORMAL_PORT) && (port_type != SWCTRL_LPORT_TRUNK_PORT))
        AMTR_MGR_RETURN(FALSE);

    *status = amtr_port_info[ifindex-1].is_learn;

    AMTR_MGR_RETURN(retval);
}
/*---------------------------------------------------------------------- */
/* The dot1dTp group (dot1dBridge 4 ) -- dot1dTp 1 ~ 3 */
/*
 *         dot1dTpLearnedEntryDiscards OBJECT-TYPE
 *             SYNTAX  Counter
 *             ::= { dot1dTp 1 }
 *----------------------------------------------------------------------
 *         dot1dTpAgingTime
 *             SYNTAX   INTEGER (10..1000000)
 *             ::= { dot1dTp 2 }
 *----------------------------------------------------------------------
 *         dot1dTpFdbTable OBJECT-TYPE
 *             SYNTAX  SEQUENCE OF Dot1dTpFdbEntry
 *             ::= { dot1dTp 3 }
 *         Dot1dTpFdbEntry
 *             INDEX   { dot1dTpFdbAddress }
 *             ::= { dot1dTpFdbTable 1 }
 *         Dot1dTpFdbEntry ::=
 *             SEQUENCE {
 *                 dot1dTpFdbAddress    MacAddress,
 *                 dot1dTpFdbPort       INTEGER,
 *                 dot1dTpFdbStatus     INTEGER
 *             }
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_GetDot1dTpAgingTime
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified aging time
 *              can be successfully retrieved. Otherwise, false is returned.
 * INPUT    :   None
 * OUTPUT   :   aging_time                  -- aging time
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dTp 2
 * ------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetDot1dTpAgingTime(UI32_T *aging_time)
{

    if (amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if (aging_time == NULL)
        AMTR_MGR_RETURN(FALSE);

    *aging_time = amtr_mgr_config_aging_time;

    AMTR_MGR_RETURN(TRUE);
} /* End of AMTR_MGR_GetDot1dTpAgingTime () */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_SetDot1dTpAgingTime
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified aging time
 *              can be successfully set. Otherwise, false is returned.
 * INPUT    :   aging_time                  -- aging time
 * OUTPUT   :   NOne
 * RETURN   :   TRUE/FALSE
 * NOTES    :   1. RFC1493/dot1dTp 2
 *              2. aging time is in [10..1000000] seconds
 * ------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_SetDot1dTpAgingTime(UI32_T aging_time)
{
    BOOL_T retval=FALSE;

    if (amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if ((aging_time < SYS_ADPT_MIN_DOT1D_TP_AGING_TIME)  ||
        (aging_time > SYS_ADPT_MAX_DOT1D_TP_AGING_TIME))
        AMTR_MGR_RETURN(FALSE);

    if (amtr_mgr_config_aging_time == aging_time)
        AMTR_MGR_RETURN(TRUE);

    if (amtr_mgr_config_aging_status == VAL_amtrMacAddrAgingStatus_enabled)
    {
        if ((retval=AMTRDRV_MGR_SetAgingTime(aging_time)))
            amtr_mgr_config_aging_time = aging_time;
    }
    else
    {
        amtr_mgr_config_aging_time = aging_time;
        retval = TRUE;
    }
    AMTR_MGR_RETURN(retval);
} /* End of AMTR_MGR_SetDot1dTpAgingTime () */

#if (SYS_CPNT_HASH_LOOKUP_DEPTH_CONFIGURABLE == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_GetHashLookupDepthFromConfig
 * ----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified hash lookup depth
 *              can be successfully retrieved. Otherwise, false is returned.
 * INPUT    :   None
 * OUTPUT   :   lookup_depth_p   -- hash lookup depth
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ----------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetHashLookupDepthFromConfig(UI32_T *lookup_depth_p)
{
    if (amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if (lookup_depth_p == NULL)
        AMTR_MGR_RETURN(FALSE);

    *lookup_depth_p = amtr_mgr_config_hash_lookup_depth;

    AMTR_MGR_RETURN(TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_GetHashLookupDepthFromChip
 * ----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified hash lookup depth
 *              can be successfully retrieved. Otherwise, false is returned.
 * INPUT    :   None
 * OUTPUT   :   lookup_depth_p   -- hash lookup depth
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ----------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetHashLookupDepthFromChip(UI32_T *lookup_depth_p)
{
    if (amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if (lookup_depth_p == NULL)
        AMTR_MGR_RETURN(FALSE);

    if (!AMTRDRV_MGR_GetHashLookupDepthFromChip(lookup_depth_p))
        AMTR_MGR_RETURN(FALSE);

    AMTR_MGR_RETURN(TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_SetMaxHashLookupLen
 *-----------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified hash lookup depth
 *              can be successfully set. Otherwise, false is returned.
 * INPUT    :   lookup_depth     -- hash lookup depth
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_SetHashLookupDepth(UI32_T lookup_depth)
{
    UI32_T i;
    BOOL_T found = FALSE;
    static const UI8_T allow_hash_depth [] =
    {
        SYS_ADPT_ALLOW_MAC_HASH_LOOKUP_DEPTH_LIST
    };

    if (amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if (amtr_mgr_config_hash_lookup_depth == lookup_depth)
        AMTR_MGR_RETURN(TRUE);

    for (i=0; i<sizeof(allow_hash_depth); i++)
    {
        if (allow_hash_depth[i] == lookup_depth)
        {
            found = TRUE;
            break;
        }
    }

    if (!found)
        AMTR_MGR_RETURN(FALSE);

    amtr_mgr_config_hash_lookup_depth = lookup_depth;

    AMTR_MGR_RETURN(TRUE);
}
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_GetDot1dTpFdbEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified forwarding database
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   tp_fdb_entry->dot1d_tp_fdb_address
 * OUTPUT   :   tp_fdb_entry -- Forwarding Database for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dTpFdbTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetDot1dTpFdbEntry(AMTR_MGR_Dot1dTpFdbEntry_T *tp_fdb_entry)
{
    AMTR_TYPE_AddrEntry_T    addr_entry;
    /* BODY
     */
    if (amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if ((tp_fdb_entry == 0) ||  ((*tp_fdb_entry).dot1d_tp_fdb_address == 0))
        AMTR_MGR_RETURN(FALSE);

    addr_entry.vid = 0;
    amtrmgr_memcpy (addr_entry.mac, (*tp_fdb_entry).dot1d_tp_fdb_address, AMTR_TYPE_MAC_LEN);

    if (!AMTR_MGR_GetNextMVAddrEntry(&addr_entry, AMTR_MGR_GET_MIB_ALL_ADDRESS))
        AMTR_MGR_RETURN(FALSE);

    if (amtrmgr_memcmp(addr_entry.mac, (*tp_fdb_entry).dot1d_tp_fdb_address, AMTR_TYPE_MAC_LEN) != 0)
        AMTR_MGR_RETURN(FALSE);

    (*tp_fdb_entry).dot1d_tp_fdb_port = addr_entry.ifindex;

    switch (addr_entry.source)
    {
        case AMTR_TYPE_ADDRESS_SOURCE_LEARN:
        case AMTR_TYPE_ADDRESS_SOURCE_SECURITY: /* source=security is not defined in the standard MIB, but is equal to source=learn */
        case AMTR_TYPE_ADDRESS_SOURCE_MLAG: /* nonstandard */
            (*tp_fdb_entry).dot1d_tp_fdb_status = VAL_dot1dTpFdbStatus_learned;
            break;
        case AMTR_TYPE_ADDRESS_SOURCE_CONFIG:
            (*tp_fdb_entry).dot1d_tp_fdb_status = VAL_dot1dTpFdbStatus_mgmt;
            break;
        case AMTR_TYPE_ADDRESS_SOURCE_SELF:
            (*tp_fdb_entry).dot1d_tp_fdb_status = VAL_dot1dTpFdbStatus_self;
            break;
        case AMTR_TYPE_ADDRESS_SOURCE_INVALID:
            (*tp_fdb_entry).dot1d_tp_fdb_status = VAL_dot1dTpFdbStatus_invalid;
            break;
        default:
            (*tp_fdb_entry).dot1d_tp_fdb_status = VAL_dot1dTpFdbStatus_other;
            break;
    }
    AMTR_MGR_RETURN(TRUE);
} /* End of AMTR_MGR_GetDot1dTpFdbEntry () */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_GetNextDot1dTpFdbEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified forwarding database
 *              entry info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   tp_fdb_entry -- Forwarding Database for Transparent Bridges
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dTpFdbTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetNextDot1dTpFdbEntry(AMTR_MGR_Dot1dTpFdbEntry_T *tp_fdb_entry)
{
    AMTR_TYPE_AddrEntry_T    addr_entry;

    /* BODY
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if ((tp_fdb_entry == NULL) || (tp_fdb_entry->dot1d_tp_fdb_address == 0))
        return FALSE;

    addr_entry.vid = 0;
    amtrmgr_memcpy (addr_entry.mac, (*tp_fdb_entry).dot1d_tp_fdb_address, AMTR_TYPE_MAC_LEN);

    /* Return only when we 1. get a different mac or 2. reach the end
     */
    do
    {
        if (!AMTR_MGR_GetNextMVAddrEntry(&addr_entry, AMTR_MGR_GET_MIB_ALL_ADDRESS))
            AMTR_MGR_RETURN(FALSE);

    }while(amtrmgr_memcmp(addr_entry.mac, (*tp_fdb_entry).dot1d_tp_fdb_address, AMTR_TYPE_MAC_LEN) == 0);

    amtrmgr_memcpy ((*tp_fdb_entry).dot1d_tp_fdb_address, addr_entry.mac, AMTR_TYPE_MAC_LEN);
    (*tp_fdb_entry).dot1d_tp_fdb_port = addr_entry.ifindex;

    switch (addr_entry.source)
    {
        case AMTR_TYPE_ADDRESS_SOURCE_LEARN:
        case AMTR_TYPE_ADDRESS_SOURCE_SECURITY: /* source=security is not defined in the standard MIB, but is equal to source=learn */
        case AMTR_TYPE_ADDRESS_SOURCE_MLAG: /* nonstandard */
            (*tp_fdb_entry).dot1d_tp_fdb_status = VAL_dot1dTpFdbStatus_learned;
            break;
        case AMTR_TYPE_ADDRESS_SOURCE_CONFIG :
            (*tp_fdb_entry).dot1d_tp_fdb_status = VAL_dot1dTpFdbStatus_mgmt;
            break;
        case AMTR_TYPE_ADDRESS_SOURCE_SELF:
            (*tp_fdb_entry).dot1d_tp_fdb_status = VAL_dot1dTpFdbStatus_self;
            break;
        case AMTR_TYPE_ADDRESS_SOURCE_INVALID:
            (*tp_fdb_entry).dot1d_tp_fdb_status = VAL_dot1dTpFdbStatus_invalid;
            break;
        default:
            (*tp_fdb_entry).dot1d_tp_fdb_status = VAL_dot1dTpFdbStatus_other;
            break;
    }
    AMTR_MGR_RETURN(TRUE);
} /* End of AMTR_MGR_GetNextDot1dTpFdbEntry () */

/*---------------------------------------------------------------------- */
/* The Static (Destination-Address Filtering) Database (dot1dBridge 5) */
/*
 *         Dot1dStaticEntry
 *             INDEX   { dot1dStaticAddress, dot1dStaticReceivePort }
 *             ::= { dot1dStaticTable 1 }
 *
 *         Dot1dStaticEntry ::=
 *             SEQUENCE {
 *                 dot1dStaticAddress           MacAddress,
 *                 dot1dStaticReceivePort       INTEGER,
 *                 dot1dStaticAllowedToGoTo     OCTET STRING,
 *                 dot1dStaticStatus            INTEGER
 *             }
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_GetDot1dStaticEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified static entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   static_entry->dot1d_static_address
 * OUTPUT   :   static_entry                  -- static entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dStaticTable 1.
 * ------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetDot1dStaticEntry(AMTR_MGR_Dot1dStaticEntry_T *static_entry)
{
    AMTR_TYPE_AddrEntry_T    addr_entry;

    /* BODY */
    if (amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if ((static_entry == NULL) ||  ((*static_entry).dot1d_static_address == 0))
        AMTR_MGR_RETURN(FALSE);

    addr_entry.vid = 0;
    amtrmgr_memcpy (addr_entry.mac, (*static_entry).dot1d_static_address, AMTR_TYPE_MAC_LEN);

    do {
        if (!AMTR_MGR_GetNextMVAddrEntry(&addr_entry, AMTR_MGR_GET_MIB_ALL_ADDRESS))
            AMTR_MGR_RETURN(FALSE);

        if (amtrmgr_memcmp(addr_entry.mac, (*static_entry).dot1d_static_address, AMTR_TYPE_MAC_LEN) != 0)
            AMTR_MGR_RETURN(FALSE);
        /* don't need to convert attribute
         * AMTR_MGR_CONVERT_ATTRIBUTE_TO_MIB_VALUE(&addr_entry);
         */
    } while ((addr_entry.life_time!= VAL_dot1dStaticStatus_other) &&
             (addr_entry.life_time != VAL_dot1dStaticStatus_permanent) &&
             (addr_entry.life_time != VAL_dot1dStaticStatus_deleteOnReset));

    (*static_entry).dot1d_static_receive_port = 0;
    AMTR_MGR_LPortToOctet ((*static_entry).dot1d_static_allowed_to_go_to, addr_entry.ifindex);
    (*static_entry).dot1d_static_status = addr_entry.life_time;
    AMTR_MGR_RETURN(TRUE);
} /* End of AMTR_MGR_GetDot1dStaticEntry () */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_GetNextDot1dStaticEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified static entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   static_entry->dot1d_static_address
 * OUTPUT   :   static_entry                  -- static entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC1493/dot1dStaticTable 1.
 * ------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetNextDot1dStaticEntry(AMTR_MGR_Dot1dStaticEntry_T *static_entry)
{
    AMTR_TYPE_AddrEntry_T    addr_entry;

    /* BODY
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if ((static_entry == NULL) ||  ((*static_entry).dot1d_static_address == 0))
        AMTR_MGR_RETURN(FALSE);

    /* static counter doesn't include under create entry.
        If static counter is 0, OM may have under create entry.
    if (AMTRDRV_OM_GetTotalStaticCounter()== 0)
        return FALSE;
    */

    addr_entry.vid = 0;
    amtrmgr_memcpy (addr_entry.mac, (*static_entry).dot1d_static_address, AMTR_TYPE_MAC_LEN);

    do
    {
        /* we return only when we 1. get a different mac or 2. reach the end
         */
        do
        {
            if (!AMTR_MGR_GetNextMVAddrEntry(&addr_entry, AMTR_MGR_GET_MIB_ALL_ADDRESS))
                AMTR_MGR_RETURN(FALSE);

        }while(amtrmgr_memcmp(addr_entry.mac, (*static_entry).dot1d_static_address, AMTR_TYPE_MAC_LEN) == 0);
    }while ((addr_entry.life_time!= VAL_dot1dStaticStatus_other) &&
            (addr_entry.life_time != VAL_dot1dStaticStatus_permanent) &&
            (addr_entry.life_time != VAL_dot1dStaticStatus_deleteOnReset)) ;

    amtrmgr_memcpy ((*static_entry).dot1d_static_address, addr_entry.mac, AMTR_TYPE_MAC_LEN);
    (*static_entry).dot1d_static_receive_port = 0;
    AMTR_MGR_LPortToOctet ((*static_entry).dot1d_static_allowed_to_go_to, addr_entry.ifindex);
    (*static_entry).dot1d_static_status = addr_entry.life_time;

    AMTR_MGR_RETURN(TRUE);
} /* End of AMTR_MGR_GetNextDot1dStaticEntry () */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_SetDot1dStaticAddress
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the static address information.
 * INPUT    :   UI8_T old_mac -- the original mac address (key)
 *              UI8_T new_mac -- the new mac to replace original mac
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * REF      :   RFC-1493/dot1dStaticEntry 1
 * ------------------------------------------------------------------------
 */
BOOL_T  AMTR_MGR_SetDot1dStaticAddress(UI8_T *old_mac, UI8_T *new_mac)
{
    AMTR_TYPE_AddrEntry_T    addr_entry;

    /* BODY
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if ((old_mac == NULL) || (new_mac == NULL))
        AMTR_MGR_RETURN(FALSE);

    addr_entry.vid = 1;
    amtrmgr_memcpy (addr_entry.mac, old_mac, AMTR_TYPE_MAC_LEN);

    if (AMTR_MGR_GetExactAddrEntry(&addr_entry))
    {
        if ((addr_entry.life_time== VAL_dot1dStaticStatus_other) ||
            (addr_entry.life_time == VAL_dot1dStaticStatus_permanent) ||
            (addr_entry.life_time == VAL_dot1dStaticStatus_deleteOnReset))
        {
            amtrmgr_memcpy (addr_entry.mac, new_mac, AMTR_TYPE_MAC_LEN);
            addr_entry.action=AMTR_TYPE_ADDRESS_ACTION_FORWARDING;
            addr_entry.source=AMTR_TYPE_ADDRESS_SOURCE_CONFIG;

            if(AMTR_MGR_SetAddrEntry(&addr_entry))
                AMTR_MGR_RETURN(TRUE);

        }
    }
    AMTR_MGR_RETURN(FALSE);
} /* End of AMTR_MGR_SetDot1dStaticAddress () */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_SetDot1dStaticReceivePort
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the static receive port information.
 * INPUT    :   UI8_T mac               -- the mac address (key)
 *              UI32_T receive_port     -- the receive port number
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * REF      :   RFC-1493/dot1dStaticEntry 2
 * ------------------------------------------------------------------------
 */
BOOL_T  AMTR_MGR_SetDot1dStaticReceivePort(UI8_T *mac, UI32_T receive_port)
{
    AMTR_TYPE_AddrEntry_T    addr_entry;

    /* BODY
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if (mac == NULL||IS_MAC_INVALID(mac)||(receive_port != 0))
        AMTR_MGR_RETURN(FALSE);

    addr_entry.vid = 1;
    amtrmgr_memcpy (addr_entry.mac, mac, AMTR_TYPE_MAC_LEN);

    if (AMTR_MGR_GetExactAddrEntry(&addr_entry))
    {
        if ((addr_entry.life_time== VAL_dot1dStaticStatus_other) ||
            (addr_entry.life_time == VAL_dot1dStaticStatus_permanent) ||
            (addr_entry.life_time == VAL_dot1dStaticStatus_deleteOnReset))
            AMTR_MGR_RETURN(TRUE);
    }
    AMTR_MGR_RETURN(FALSE);
} /* End of AMTR_MGR_SetDot1dStaticReceivePort () */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_SetDot1dStaticAllowedToGoTo
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the static allowed to go to information.
 * INPUT    :   UI8_T mac               -- the mac address (key)
 *              UI8_T allow_to_go_to    -- the set of ports(often only one bit is on)
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   allow_to_go_to == addr_entry.ifindex
 * REF      :   RFC-1493/dot1dStaticEntry 3
 * ------------------------------------------------------------------------
 */
BOOL_T  AMTR_MGR_SetDot1dStaticAllowedToGoTo(UI8_T *mac, UI8_T *allowed_to_go_to)
{
    AMTR_TYPE_AddrEntry_T    addr_entry;

    /* BODY */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if (mac == NULL || allowed_to_go_to == NULL)
        AMTR_MGR_RETURN(FALSE);

    addr_entry.vid = 1;
    amtrmgr_memcpy (addr_entry.mac, mac, AMTR_TYPE_MAC_LEN);

    if (!AMTR_MGR_GetExactAddrEntry(&addr_entry))
        addr_entry.life_time= VAL_dot1dStaticStatus_permanent;

    if (addr_entry.life_time == VAL_dot1dStaticStatus_deleteOnTimeout)
        addr_entry.life_time = VAL_dot1dStaticStatus_other;

    if ((addr_entry.ifindex = AMTR_MGR_OctetToLPort (allowed_to_go_to , SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST))== 0)
        AMTR_MGR_RETURN(FALSE);

    addr_entry.action=AMTR_TYPE_ADDRESS_ACTION_FORWARDING;
    addr_entry.source=AMTR_TYPE_ADDRESS_SOURCE_CONFIG;
    if (AMTR_MGR_SetAddrEntry (&addr_entry))
        AMTR_MGR_RETURN(TRUE);

    AMTR_MGR_RETURN(FALSE);
} /* End of AMTR_MGR_SetDot1dStaticAllowedToGoTo () */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_SetDot1dStaticStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set/Create the static status information.
 * INPUT    :   UI8_T mac               -- the mac address (key)
 *              UI32_T status           -- VAL_dot1dStaticStatus_other
 *                                         VAL_dot1dStaticStatus_invalid
 *                                         VAL_dot1dStaticStatus_permanent
 *                                         VAL_dot1dStaticStatus_deleteOnReset
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * REF      :   RFC-1493/dot1dStaticEntry 4
 * ------------------------------------------------------------------------
 */
BOOL_T  AMTR_MGR_SetDot1dStaticStatus(UI8_T *mac, UI32_T status)
{
    AMTR_TYPE_AddrEntry_T    addr_entry;
    BOOL_T retval;
    /* BODY
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if (mac == NULL)
        AMTR_MGR_RETURN(FALSE);

    if ((status != AMTR_TYPE_ADDRESS_LIFETIME_OTHER) &&
        (status != AMTR_TYPE_ADDRESS_LIFETIME_INVALID)&&
        (status != AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT) &&
        (status != AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET))
        AMTR_MGR_RETURN(FALSE);

    addr_entry.vid = 1;
    amtrmgr_memcpy (addr_entry.mac, mac, AMTR_TYPE_MAC_LEN);

    if (AMTR_MGR_GetExactAddrEntry(&addr_entry))
    {
        if (addr_entry.life_time == status)
            AMTR_MGR_RETURN(TRUE);
        else if (status== VAL_dot1dStaticStatus_invalid){
            retval = AMTR_MGR_DeleteAddr(addr_entry.vid, addr_entry.mac);
            AMTR_MGR_RETURN(retval);
        }else
        {
            addr_entry.action=AMTR_TYPE_ADDRESS_ACTION_FORWARDING;
            addr_entry.source=AMTR_TYPE_ADDRESS_SOURCE_CONFIG;
            addr_entry.life_time=status;
            retval = AMTR_MGR_SetAddrEntry ( &addr_entry);
            AMTR_MGR_RETURN(retval);
        }
    }

    /* Can't get this entry, return TRUE
     */
    if (addr_entry.life_time== VAL_dot1dStaticStatus_invalid)
        AMTR_MGR_RETURN(TRUE);

    /* we don't know "lport".
     * So we set "lport=0", let the entry just be kept in Hisam table.
     * It won't be set to chip!
     */
    addr_entry.ifindex = 0;
    addr_entry.action=AMTR_TYPE_ADDRESS_ACTION_FORWARDING;
    addr_entry.source= AMTR_TYPE_ADDRESS_SOURCE_CONFIG;
    addr_entry.life_time=status;
    retval = AMTR_MGR_SetAddrEntry(&addr_entry);
    AMTR_MGR_RETURN(retval);
} /* End of AMTR_MGR_SetDot1dStaticStatus () */

/*---------------------------------------------------------------------- */
/* the current Filtering Database Table (the dot1qTp group 1) */
/*
 *       INDEX   { dot1qFdbId }
 *       Dot1qFdbEntry ::=
 *       SEQUENCE {
 *           dot1qFdbId             Unsigned32,
 *           dot1qFdbDynamicCount   Counter32
 *       }
 */
/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetDot1qFdbEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the filtering database entry info
 * INPUT   : dot1q_fdb_entry->dot1q_fdb_id  - The identity of this Filtering Database
 * OUTPUT  : dot1q_fdb_entry                - The  Filtering Database entry
 * RETURN  : TRUE/FALSE
 * NOTE    : RFC2674Q/dot1qTp 1
 *------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_GetDot1qFdbEntry(AMTR_MGR_Dot1qFdbEntry_T *dot1q_fdb_entry)
{
    UI32_T vid;
    /* BODY
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if (dot1q_fdb_entry == NULL)
        AMTR_MGR_RETURN(FALSE);

    vid = (*dot1q_fdb_entry).dot1q_fdb_id;

#ifdef SYS_HWCFG_NO_VID_FIELD_IN_ADDR_TBL
    if (vid != 1)
#else
    if (!VLAN_OM_IsVlanExisted(vid))
#endif
        AMTR_MGR_RETURN(FALSE);

    (*dot1q_fdb_entry).dot1q_fdb_dynamic_count = AMTR_OM_GetDynCounterByVid(vid);
    AMTR_MGR_RETURN(TRUE);
} /* end AMTR_MGR_GetDot1qFdbEntry */

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetNextDot1qFdbEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next filtering database entry info
 * INPUT   : dot1q_fdb_entry->dot1q_fdb_id  - The identity of this Filtering Database
 * OUTPUT  : dot1q_fdb_entry                - The  Filtering Database entry
 * RETURN  : TRUE/FALSE
 * NOTE    : RFC2674Q/dot1qTp 1
 *------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_GetNextDot1qFdbEntry(AMTR_MGR_Dot1qFdbEntry_T *dot1q_fdb_entry)
{
    UI32_T vid;
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_info;
    UI32_T  vlan_ifindex=0;
    /* BODY */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if (dot1q_fdb_entry == 0)
        AMTR_MGR_RETURN(FALSE);

    vid = (*dot1q_fdb_entry).dot1q_fdb_id+1;

#ifdef SYS_HWCFG_NO_VID_FIELD_IN_ADDR_TBL
    if (vid != 1)
        AMTR_MGR_RETURN(FALSE);
#else
    /*EPR:ES3628BT-FLF-ZZ-00114
     *Problem: Q-BRIDGE-MIB: OID (1.3.6.1.2.1.17.7.1.2.1.1.2) can't be get.
     *Root Cause: The MIB getnext index use error to VID,need use the VID to the
     *            getnext index
     *Solution: Add function for specify handle get the MIB index
     *Modify file: es3636a_superset2.0.c,rfc_2674q.c
     *Fixed by:DanXie
     */
#if 0 /* DanXie, Thursday, December 25, 2008 1:35:59 */
    if(TRUE!=VLAN_OM_ConvertToIfindex((*dot1q_fdb_entry).dot1q_fdb_id,&vlan_ifindex))
        AMTR_MGR_RETURN(FALSE);
#else
    VLAN_VID_CONVERTTO_IFINDEX((*dot1q_fdb_entry).dot1q_fdb_id,vlan_ifindex);
#endif /* #if 0 */
    vlan_info.dot1q_vlan_index = vlan_ifindex;
    if(TRUE !=VLAN_OM_GetNextVlanEntry(&vlan_info))
        AMTR_MGR_RETURN(FALSE);
#endif

    (*dot1q_fdb_entry).dot1q_fdb_id = vlan_info.dot1q_vlan_fdb_id ;
    (*dot1q_fdb_entry).dot1q_fdb_dynamic_count = AMTR_OM_GetDynCounterByVid(vlan_info.dot1q_vlan_fdb_id);
    AMTR_MGR_RETURN(TRUE);
} /* end AMTR_MGR_GetNextDot1qFdbEntry */


/*---------------------------------------------------------------------- */
/* (the dot1qTp group 2) */
/*
 *      INDEX   { dot1qFdbId, dot1qTpFdbAddress }
 *      Dot1qTpFdbEntry ::=
 *          SEQUENCE {
 *              dot1qTpFdbAddress  MacAddress,
 *              dot1qTpFdbPort     INTEGER,
 *              dot1qTpFdbStatus   INTEGER
 *          }
 */
/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetDot1qTpFdbEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the dot1qTpFdbEntry info
 * INPUT   : dot1q_tp_fdb_entry->dot1q_fdb_id  - vlan id
 *           dot1q_tp_fdb_entry->dot1q_tp_fdb_address - mac address
 * OUTPUT  : dot1q_tp_fdb_entry                - The dot1qTpFdbEntry info
 * RETURN  : TRUE/FALSE
 * NOTE    : RFC2674Q/dot1qTp 2
 *           dot1q_tp_fdb_entry->dot1q_tp_fdb_status == addr_entry->source
 *------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_GetDot1qTpFdbEntry(UI32_T dot1q_fdb_id, AMTR_MGR_Dot1qTpFdbEntry_T *dot1q_tp_fdb_entry)
{
    AMTR_TYPE_AddrEntry_T    addr_entry;

    /* BODY
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if ((dot1q_tp_fdb_entry == NULL) || ((*dot1q_tp_fdb_entry).dot1q_tp_fdb_address == NULL))
        AMTR_MGR_RETURN(FALSE);
#ifdef SYS_HWCFG_NO_VID_FIELD_IN_ADDR_TBL
    if (dot1q_fdb_id != 1)
        AMTR_MGR_RETURN(FALSE);
#else
    if (!VLAN_OM_IsVlanExisted(dot1q_fdb_id))
        AMTR_MGR_RETURN(FALSE);
#endif

    addr_entry.vid = dot1q_fdb_id;
    amtrmgr_memcpy (addr_entry.mac, (*dot1q_tp_fdb_entry).dot1q_tp_fdb_address, AMTR_TYPE_MAC_LEN);

    if (!AMTR_MGR_GetExactAddrEntry(&addr_entry))
        AMTR_MGR_RETURN(FALSE);

    (*dot1q_tp_fdb_entry).dot1q_tp_fdb_port = addr_entry.ifindex;

    switch (addr_entry.source)
    {
        case AMTR_TYPE_ADDRESS_SOURCE_LEARN :
        case AMTR_TYPE_ADDRESS_SOURCE_SECURITY: /* source=security is not defined in the standard MIB, but is equal to source=learn */
        case AMTR_TYPE_ADDRESS_SOURCE_MLAG: /* nonstandard */
            (*dot1q_tp_fdb_entry).dot1q_tp_fdb_status = VAL_dot1qTpFdbStatus_learned;
            break;
        case AMTR_TYPE_ADDRESS_SOURCE_CONFIG :
            (*dot1q_tp_fdb_entry).dot1q_tp_fdb_status = VAL_dot1qTpFdbStatus_mgmt;
            break;
        case AMTR_TYPE_ADDRESS_SOURCE_SELF :
            (*dot1q_tp_fdb_entry).dot1q_tp_fdb_status = VAL_dot1qTpFdbStatus_self;
            break;
        case AMTR_TYPE_ADDRESS_SOURCE_INVALID:
            (*dot1q_tp_fdb_entry).dot1q_tp_fdb_status = VAL_dot1qTpFdbStatus_invalid;
            break;
        default:
            (*dot1q_tp_fdb_entry).dot1q_tp_fdb_status = VAL_dot1qTpFdbStatus_other;
            break;
    }
    AMTR_MGR_RETURN(TRUE);
} /* End of AMTR_MGR_GetDot1qTpFdbEntry () */

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetNextDot1qTpFdbEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next dot1qTpFdbEntry info
 * INPUT   : dot1q_tp_fdb_entry->dot1q_fdb_id         - vlan id
 *           dot1q_tp_fdb_entry->dot1q_tp_fdb_address - mac address
 * OUTPUT  : dot1q_tp_fdb_entry->dot1q_fdb_id         - vlan id
 *           dot1q_tp_fdb_entry->dot1q_tp_fdb_address - mac address   - The dot1qTpFdbEntry info
 * RETURN  : TRUE/FALSE
 * NOTE    : RFC2674Q/dot1qTp 2
 *           dot1q_tp_fdb_entry->dot1q_tp_fdb_status == addr_entry->source
 *------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_GetNextDot1qTpFdbEntry(UI32_T *dot1q_fdb_id, AMTR_MGR_Dot1qTpFdbEntry_T *dot1q_tp_fdb_entry)
{
    AMTR_TYPE_AddrEntry_T    addr_entry;

    /* BODY
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if ((dot1q_tp_fdb_entry == NULL) || (dot1q_fdb_id == NULL))
        AMTR_MGR_RETURN(FALSE);

#ifdef SYS_HWCFG_NO_VID_FIELD_IN_ADDR_TBL
    addr_entry.vid = 1;
#else
    addr_entry.vid = *dot1q_fdb_id;
#endif
    amtrmgr_memcpy (addr_entry.mac, (*dot1q_tp_fdb_entry).dot1q_tp_fdb_address, AMTR_TYPE_MAC_LEN);

    if (!AMTR_MGR_GetNextVMAddrEntry(&addr_entry, AMTR_MGR_GET_MIB_ALL_ADDRESS))
        AMTR_MGR_RETURN(FALSE);

    *dot1q_fdb_id = addr_entry.vid;
    amtrmgr_memcpy ((*dot1q_tp_fdb_entry).dot1q_tp_fdb_address, addr_entry.mac, AMTR_TYPE_MAC_LEN);
    (*dot1q_tp_fdb_entry).dot1q_tp_fdb_port = addr_entry.ifindex;

    switch (addr_entry.source)
    {
        case AMTR_TYPE_ADDRESS_SOURCE_LEARN :
        case AMTR_TYPE_ADDRESS_SOURCE_SECURITY: /* source=security is not defined in the standard MIB, but is equal to source=learn */
        case AMTR_TYPE_ADDRESS_SOURCE_MLAG: /* nonstandard */
            (*dot1q_tp_fdb_entry).dot1q_tp_fdb_status = VAL_dot1qTpFdbStatus_learned;
            break;
        case AMTR_TYPE_ADDRESS_SOURCE_CONFIG :
            (*dot1q_tp_fdb_entry).dot1q_tp_fdb_status = VAL_dot1qTpFdbStatus_mgmt;
            break;
        case AMTR_TYPE_ADDRESS_SOURCE_SELF :
            (*dot1q_tp_fdb_entry).dot1q_tp_fdb_status = VAL_dot1qTpFdbStatus_self;
            break;
        case AMTR_TYPE_ADDRESS_SOURCE_INVALID:
            (*dot1q_tp_fdb_entry).dot1q_tp_fdb_status = VAL_dot1qTpFdbStatus_invalid;
            break;
        default:
            (*dot1q_tp_fdb_entry).dot1q_tp_fdb_status = VAL_dot1qTpFdbStatus_other;
            break;
    }
    AMTR_MGR_RETURN(TRUE);
} /* End of AMTR_MGR_GetNextDot1qTpFdbEntry () */

/*---------------------------------------------------------------------- */
/* The Static (Destination-Address Filtering) Database (dot1qStatic 1) */
/*
 *             INDEX   { dot1qFdbId, dot1qStaticUnicastAddress, dot1qStaticUnicastReceivePort }
 *             ::= { dot1qStaticUnicastTable 1 }
 *
 *         Dot1qStaticUnicastEntry ::=
 *             SEQUENCE {
 *                 dot1qStaticUnicastAddress           MacAddress,
 *                 dot1qStaticUnicastReceivePort       INTEGER,
 *                 dot1qStaticUnicastAllowedToGoTo     OCTET STRING,
 *                 dot1qStaticUnicastStatus            INTEGER
 *             }
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_GetDot1qStaticUnicastEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified static entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   dot1q_fdb_id                -- vlan id
 *              static_unitcast_entry->dot1q_static_unicast_address
 * OUTPUT   :   static_unitcast_entry       -- static entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC2674q/dot1qStaticUnicastTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetDot1qStaticUnicastEntry(UI32_T dot1q_fdb_id,
                                           AMTR_MGR_Dot1qStaticUnicastEntry_T *static_unitcast_entry)
{
    AMTR_TYPE_AddrEntry_T    addr_entry;

    /* BODY
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if (static_unitcast_entry == NULL)
        AMTR_MGR_RETURN(FALSE);

#ifdef SYS_HWCFG_NO_VID_FIELD_IN_ADDR_TBL
    addr_entry.vid = 1;
#else
    addr_entry.vid = dot1q_fdb_id;
#endif
    amtrmgr_memcpy (addr_entry.mac, (*static_unitcast_entry).dot1q_static_unicast_address, AMTR_TYPE_MAC_LEN);

    if (!AMTR_MGR_GetExactAddrEntry(&addr_entry))
        AMTR_MGR_RETURN(FALSE);

    if ((addr_entry.life_time!= VAL_dot1qStaticUnicastStatus_other) &&
        (addr_entry.life_time != VAL_dot1qStaticUnicastStatus_permanent) &&
        (addr_entry.life_time != VAL_dot1qStaticUnicastStatus_deleteOnReset))
        AMTR_MGR_RETURN(FALSE);

    (*static_unitcast_entry).dot1q_static_unicast_receive_port = 0;
    AMTR_MGR_LPortToOctet ((*static_unitcast_entry).dot1q_static_unicast_allowed_to_go_to, addr_entry.ifindex);
    (*static_unitcast_entry).dot1q_static_unicast_status = addr_entry.life_time;
    AMTR_MGR_RETURN(TRUE);
} /* End of AMTR_MGR_GetDot1qStaticUnicastEntry () */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_GetNextDot1qStaticUnicastEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next specified static entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   dot1q_fdb_id                -- vlan id
 *              static_unitcast_entry->dot1q_static_unicast_address
 * OUTPUT   :   static_unitcast_entry       -- static entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   RFC2674q/dot1qStaticUnicastTable 1
 * ------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetNextDot1qStaticUnicastEntry(UI32_T *dot1q_fdb_id,
                                               AMTR_MGR_Dot1qStaticUnicastEntry_T *static_unitcast_entry)
{
    AMTR_TYPE_AddrEntry_T    addr_entry;
    /* BODY
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if ((static_unitcast_entry == NULL) || (dot1q_fdb_id == NULL))
        AMTR_MGR_RETURN(FALSE);

    /* static counter doesn't include under create entry.
        If static counter is 0, OM may have under create entry.
    if (AMTRDRV_OM_GetTotalStaticCounter()== 0)
        return FALSE;
    */
#ifdef SYS_HWCFG_NO_VID_FIELD_IN_ADDR_TBL
    addr_entry.vid = 1;
#else
    addr_entry.vid = *dot1q_fdb_id;
#endif
    amtrmgr_memcpy (addr_entry.mac, (*static_unitcast_entry).dot1q_static_unicast_address, AMTR_TYPE_MAC_LEN);

    do{
        if (!AMTR_MGR_GetNextMVAddrEntry(&addr_entry, AMTR_MGR_GET_MIB_ALL_ADDRESS))
            AMTR_MGR_RETURN(FALSE);
    }while ((addr_entry.life_time!= AMTR_TYPE_ADDRESS_LIFETIME_OTHER) &&
            (addr_entry.life_time != AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT) &&
            (addr_entry.life_time != AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET)) ;

    *dot1q_fdb_id = addr_entry.vid;
    amtrmgr_memcpy ((*static_unitcast_entry).dot1q_static_unicast_address, addr_entry.mac, AMTR_TYPE_MAC_LEN);
    (*static_unitcast_entry).dot1q_static_unicast_receive_port = 0;
    AMTR_MGR_LPortToOctet ((*static_unitcast_entry).dot1q_static_unicast_allowed_to_go_to, addr_entry.ifindex);
    (*static_unitcast_entry).dot1q_static_unicast_status = addr_entry.life_time;
    AMTR_MGR_RETURN(TRUE);
}/* End of AMTR_MGR_GetNextDot1qStaticUnicastEntry () */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_SetDot1qStaticUnicastAllowedToGoTo
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the static allowed to go to information.
 * INPUT    :   UI32_T vid              -- vlan id
 *              UI8_T mac               -- the mac address (key)
 *              UI8_T allow_to_go_to    -- the set of ports
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * REF      :   RFC2674q/dot1qStaticUnicastEntry 3
 * ------------------------------------------------------------------------
 */
BOOL_T  AMTR_MGR_SetDot1qStaticUnicastAllowedToGoTo(UI32_T vid, UI8_T *mac, UI8_T *allowed_to_go_to)
{
    AMTR_TYPE_AddrEntry_T    addr_entry;
    BOOL_T                   retval;
    /* BODY
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if (mac == NULL || allowed_to_go_to == NULL)
        AMTR_MGR_RETURN(FALSE);

#ifdef SYS_HWCFG_NO_VID_FIELD_IN_ADDR_TBL
    addr_entry.vid = 1;
#else
    addr_entry.vid = vid;
#endif
    amtrmgr_memcpy (addr_entry.mac, mac, AMTR_TYPE_MAC_LEN);

    if (!AMTR_MGR_GetExactAddrEntry(&addr_entry))
        addr_entry.life_time= VAL_dot1dStaticStatus_other;/*attribute = other = under create*/
    /* Because the entry is not exist in Hash table, so we don't the attribute about this mac
     * So the information is not enough, the attribute is other
     */

    if (addr_entry.life_time == VAL_dot1dStaticStatus_deleteOnTimeout)
        addr_entry.life_time = VAL_dot1dStaticStatus_other;

    if ((addr_entry.ifindex= AMTR_MGR_OctetToLPort (allowed_to_go_to , SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST))
         == 0)
        AMTR_MGR_RETURN(FALSE);

    addr_entry.source=AMTR_TYPE_ADDRESS_SOURCE_CONFIG;
    addr_entry.action=AMTR_TYPE_ADDRESS_ACTION_FORWARDING;
    retval = AMTR_MGR_SetAddrEntry (&addr_entry);
    AMTR_MGR_RETURN(retval);
}/* End of AMTR_MGR_SetDot1qStaticUnicastAllowedToGoTo () */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_SetDot1qStaticUnicastStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set/Create the static status information.
 * INPUT    :   UI32_T vid              -- vlan id
 *              UI8_T mac               -- the mac address (key)
 *              UI32_T status           -- VAL_dot1qStaticUnicastStatus_other
 *                                         VAL_dot1qStaticUnicastStatus_invalid
 *                                         VAL_dot1qStaticUnicastStatus_permanent
 *                                         VAL_dot1qStaticUnicastStatus_deleteOnReset
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * REF      :   RFC2674q/dot1qStaticUnicastEntry 4
 * ------------------------------------------------------------------------
 */
BOOL_T  AMTR_MGR_SetDot1qStaticUnicastStatus(UI32_T vid, UI8_T *mac, UI32_T status)
{
    AMTR_TYPE_AddrEntry_T    addr_entry;
    BOOL_T  retval;
    /* BODY
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if (mac == NULL)
        AMTR_MGR_RETURN(FALSE);

    if ((status != AMTR_TYPE_ADDRESS_LIFETIME_OTHER) &&
        (status != AMTR_TYPE_ADDRESS_LIFETIME_INVALID)&&
        (status != AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT) &&
        (status != AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET))
        AMTR_MGR_RETURN(FALSE);

#ifdef SYS_HWCFG_NO_VID_FIELD_IN_ADDR_TBL
    addr_entry.vid = 1;
#else
    addr_entry.vid = vid;
#endif
    amtrmgr_memcpy (addr_entry.mac, mac, AMTR_TYPE_MAC_LEN);

    if (AMTR_MGR_GetExactAddrEntry(&addr_entry))
    {
        if (addr_entry.life_time== status)
            AMTR_MGR_RETURN(TRUE);
        else if (status== VAL_dot1dStaticStatus_invalid)
            retval = AMTR_MGR_DeleteAddr(addr_entry.vid, addr_entry.mac);
        else
        {
            addr_entry.life_time=status;
            addr_entry.source=AMTR_TYPE_ADDRESS_SOURCE_CONFIG;
            addr_entry.action=AMTR_TYPE_ADDRESS_ACTION_FORWARDING;
            retval = AMTR_MGR_SetAddrEntry(&addr_entry);
        }
        AMTR_MGR_RETURN(retval);
    }

    /* Can't get this entry, return TRUE
     */
    if (addr_entry.life_time== VAL_dot1dStaticStatus_invalid)
        AMTR_MGR_RETURN(TRUE);

    addr_entry.ifindex=0;
    addr_entry.life_time=status;
    addr_entry.source=AMTR_TYPE_ADDRESS_SOURCE_CONFIG;
    addr_entry.action=AMTR_TYPE_ADDRESS_ACTION_FORWARDING;
    retval = AMTR_MGR_SetAddrEntry(&addr_entry);
    AMTR_MGR_RETURN(retval);
} /* End of AMTR_MGR_SetDot1qStaticUnicastStatus () */

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_SetLearningMode
 *------------------------------------------------------------------------------
 * Purpose  : This function will set the learning mode for whole system
 * INPUT    : UI32_T learning_mode    - VAL_dot1qConstraintType_independent (IVL)
 *                                      VAL_dot1qConstraintType_shared (SVL)
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : RFC2674q
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTR_MGR_SetLearningMode (UI32_T learning_mode)
{
    /* BODY
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    amtr_mgr_vlan_learning_mode = learning_mode;
    AMTR_MGR_RETURN(TRUE);
}

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_SetLearningMode
 *------------------------------------------------------------------------------
 * Purpose  : This function check whether port security is enabled on a specific port or not.
 * INPUT    : UI32_T ifindex    - interface index
 * OUTPUT   : None
 * RETURN   : True : enabled, False : disabled
 * NOTE     :
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTR_MGR_IsPortSecurityEnabled(UI32_T ifindex)
{
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if(IS_IFINDEX_INVALID(ifindex))
        AMTR_MGR_RETURN(FALSE);

    if (amtr_port_info[ifindex-1].protocol != AMTR_MGR_PROTOCOL_NORMAL)
    {
        /* whether the secur port is full or not
         */
#if (SYS_DFLT_L2_ADDR_PSEC_LEARN_COUNT_INCLUDE_CONFIG)
        if((AMTRDRV_OM_GetSecurityCounterByport(ifindex) + AMTRDRV_OM_GetConfigCounterByPort(ifindex))
             >= amtr_port_info[ifindex-1].learn_with_count)
            AMTR_MGR_RETURN(TRUE);
#else
        if(AMTRDRV_OM_GetSecurityCounterByport(ifindex) >= amtr_port_info[ifindex-1].learn_with_count)
            AMTR_MGR_RETURN(TRUE);
#endif
    }/*end if(prtotcol != Normal)*/
    AMTR_MGR_RETURN(FALSE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_IsPortSecurityEnableByAutoLearn
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return Ture if the port security is enabled by
 *           auto learn
 * INPUT   : ifindex        -- which port to set
 * OUTPUT  : none
 * RETURN  : TRUE/FALSE
 * NOTE    : If the status of port security is enabled by Auto Learn,
 *           1. the status of port security should not be got from GetRunning.
 *           2. the security mac should not be got form GetNextRunning.
 * -------------------------------------------------------------------------*/
BOOL_T AMTR_MGR_IsPortSecurityEnableByAutoLearn(UI32_T ifindex)
{
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if(IS_IFINDEX_INVALID(ifindex))
        AMTR_MGR_RETURN(FALSE);

    if (AMTR_MGR_IsPortSecurityEnabled(ifindex)&&
        (amtr_port_info[ifindex-1].learn_with_count!=0))
        AMTR_MGR_RETURN(TRUE);

    AMTR_MGR_RETURN(FALSE);
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AMTR_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for AMTR MGR.
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
BOOL_T AMTR_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    AMTR_MGR_IpcMsg_T *msg_p;

    if (msgbuf_p == NULL)
        AMTR_MGR_RETURN(FALSE);

    msg_p = (AMTR_MGR_IpcMsg_T*)msgbuf_p->msg_buf;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (amtr_mgr_operating_mode == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        msg_p->type.ret_bool = FALSE;
        msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
        AMTR_MGR_RETURN(TRUE);
    }

    /* dispatch IPC message and call the corresponding AMTR_MGR function
     */
    switch (msg_p->type.cmd)
    {
        case AMTR_MGR_IPC_GETRUNNINGAGINGTIME:
            msg_p->type.ret_running_cfg = AMTR_MGR_GetRunningAgingTime(
                &msg_p->data.arg_ui32);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_ui32);
            break;

        case AMTR_MGR_IPC_SETADDRENTRY:
            msg_p->type.ret_bool = AMTR_MGR_SetAddrEntry(
                &msg_p->data.arg_addr_entry);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;

#if (SYS_CPNT_MLAG == TRUE)
        case AMTR_MGR_IPC_SETADDRENTRYFORMLAG:
            msg_p->type.ret_bool = AMTR_MGR_SetAddrEntryForMlag(
                &msg_p->data.arg_addr_entry);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;
#endif

        case AMTR_MGR_IPC_DELETEADDR:
            msg_p->type.ret_bool = AMTR_MGR_DeleteAddr(
                msg_p->data.arg_grp_ui32_mac.arg_ui32,
                msg_p->data.arg_grp_ui32_mac.arg_mac);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;

#if (SYS_CPNT_MLAG == TRUE)
        case AMTR_MGR_IPC_DELETEADDRFORMLAG:
            msg_p->type.ret_bool = AMTR_MGR_DeleteAddrForMlag(
                msg_p->data.arg_grp_ui32_mac.arg_ui32,
                msg_p->data.arg_grp_ui32_mac.arg_mac);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;
#endif

        case AMTR_MGR_IPC_DELETEADDRBYLIFETIME:
            msg_p->type.ret_bool = AMTR_MGR_DeleteAddrByLifeTime(
                msg_p->data.arg_address_life_time);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;

        case AMTR_MGR_IPC_DELETEADDRBYLIFETIMEANDLPORT:
             AMTR_MGR_DeleteAddrByLifeTimeAndLPort(
                msg_p->data.arg_grp_ui32_alt.arg_ui32,
                msg_p->data.arg_grp_ui32_alt.arg_addresslifetime);
            AMTR_MGR_RETURN(FALSE);
            break;

        case AMTR_MGR_IPC_DELETEADDRBYSOURCEANDLPORT:
             AMTR_MGR_DeleteAddrBySourceAndLPort(
                msg_p->data.arg_grp_ui32_alt.arg_ui32,
                msg_p->data.arg_grp_ui32_alt.arg_addresslifetime);
            AMTR_MGR_RETURN(FALSE);
            break;

        case AMTR_MGR_IPC_DELETEADDRBYLIFETIMEANDVIDRANGEANDLPORT:
            AMTR_MGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Internal(
                msg_p->data.arg_ui32_ui32_ui16_ui16_bool.arg_ui32_1,
                msg_p->data.arg_ui32_ui32_ui16_ui16_bool.arg_ui16_1,
                msg_p->data.arg_ui32_ui32_ui16_ui16_bool.arg_ui16_2,
                msg_p->data.arg_ui32_ui32_ui16_ui16_bool.arg_ui32_2,
                msg_p->data.arg_ui32_ui32_ui16_ui16_bool.arg_bool_1);
            if (msg_p->data.arg_ui32_ui32_ui16_ui16_bool.arg_bool_1==FALSE)
            {
                AMTR_MGR_RETURN(FALSE);
            }
            else
            {
                msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            }
            break;
        case AMTR_MGR_IPC_DELETEADDRBYLIFETIMEANDMSTIDANDLPORT:
            AMTR_MGR_DeleteAddrByLifeTimeAndMstIDAndLPort(
                msg_p->data.arg_ui32_ui32_ui32.arg_ui32_1,
                msg_p->data.arg_ui32_ui32_ui32.arg_ui32_2,
                msg_p->data.arg_ui32_ui32_ui32.arg_ui32_3);
                AMTR_MGR_RETURN(FALSE);
            break;
        case AMTR_MGR_IPC_DELETEADDRBYVIDANDLPORT:
            AMTR_MGR_DeleteAddrByVidAndLPort(
                msg_p->data.arg_ui32_ui32_ui32.arg_ui32_1,
                msg_p->data.arg_ui32_ui32_ui32.arg_ui32_2);
                AMTR_MGR_RETURN(FALSE);
            break;
        case AMTR_MGR_IPC_GETEXACTADDRENTRY:
            msg_p->type.ret_bool = AMTR_MGR_GetExactAddrEntry(
                &msg_p->data.arg_addr_entry);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_addr_entry);
            break;

        case AMTR_MGR_IPC_GETEXACTADDRENTRYFROMCHIP:
            msg_p->type.ret_bool = AMTR_MGR_GetExactAddrEntryFromChip(
                &msg_p->data.arg_addr_entry);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_addr_entry);
            break;

        case AMTR_MGR_IPC_GETNEXTMVADDRENTRY:
            msg_p->type.ret_bool = AMTR_MGR_GetNextMVAddrEntry(
                &msg_p->data.arg_grp_addrentry_ui32.arg_addr_entry,
                msg_p->data.arg_grp_addrentry_ui32.arg_ui32);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_addrentry_ui32);
            break;

        case AMTR_MGR_IPC_GETNEXTVMADDRENTRY:
            msg_p->type.ret_bool = AMTR_MGR_GetNextVMAddrEntry(
                &msg_p->data.arg_grp_addrentry_ui32.arg_addr_entry,
                msg_p->data.arg_grp_addrentry_ui32.arg_ui32);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_addrentry_ui32);
            break;

        case AMTR_MGR_IPC_GETNEXTIFINDEXADDRENTRY:
            msg_p->type.ret_bool = AMTR_MGR_GetNextIfIndexAddrEntry(
                &msg_p->data.arg_grp_addrentry_ui32.arg_addr_entry,
                msg_p->data.arg_grp_addrentry_ui32.arg_ui32);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_addrentry_ui32);
            break;

        case AMTR_MGR_IPC_GETNEXTRUNNINGSTATICADDRENTRY:
            msg_p->type.ret_running_cfg = AMTR_MGR_GetNextRunningStaticAddrEntry(
                &msg_p->data.arg_addr_entry);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_addr_entry);
            break;

        case AMTR_MGR_IPC_SETINTERVENTIONENTRY:
            msg_p->type.ret_amtr_type = AMTR_MGR_SetInterventionEntry(
                msg_p->data.arg_grp_ui32_mac.arg_ui32,
                msg_p->data.arg_grp_ui32_mac.arg_mac);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;

        case AMTR_MGR_IPC_DELETEINTERVENTIONENTRY:
            msg_p->type.ret_bool = AMTR_MGR_DeleteInterventionEntry(
                msg_p->data.arg_grp_ui32_mac.arg_ui32,
                msg_p->data.arg_grp_ui32_mac.arg_mac);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;

        case AMTR_MGR_IPC_CREATEMULTICASTADDRTBLENTRY:
            msg_p->type.ret_bool = AMTR_MGR_CreateMulticastAddrTblEntry(
                msg_p->data.arg_grp_ui32_mac.arg_ui32,
                msg_p->data.arg_grp_ui32_mac.arg_mac);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;

        case AMTR_MGR_IPC_DESTROYMULTICASTADDRTBLENTRY:
            msg_p->type.ret_bool = AMTR_MGR_DestroyMulticastAddrTblEntry(
                msg_p->data.arg_grp_ui32_mac.arg_ui32,
                msg_p->data.arg_grp_ui32_mac.arg_mac);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;

        case AMTR_MGR_IPC_SETMULTICASTPORTMEMBER:
            msg_p->type.ret_bool = AMTR_MGR_SetMulticastPortMember(
                msg_p->data.arg_grp_ui32_mac_ports_tagged.arg_ui32,
                msg_p->data.arg_grp_ui32_mac_ports_tagged.arg_mac,
                msg_p->data.arg_grp_ui32_mac_ports_tagged.arg_ports,
                msg_p->data.arg_grp_ui32_mac_ports_tagged.arg_tagged);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;

        case AMTR_MGR_IPC_GETAGINGSTATUS:
            msg_p->type.ret_bool = AMTR_MGR_GetAgingStatus(
                &msg_p->data.arg_ui32);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_ui32);
            break;

        case AMTR_MGR_IPC_GETRUNNINGAGINGSTATUS:
            msg_p->type.ret_running_cfg = AMTR_MGR_GetRunningAgingStatus(
                &msg_p->data.arg_ui32);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_ui32);
            break;

        case AMTR_MGR_IPC_SETAGINGSTATUS:
            msg_p->type.ret_bool = AMTR_MGR_SetAgingStatus(
                msg_p->data.arg_ui32);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;

        case AMTR_MGR_IPC_GETDOT1DTPAGINGTIME:
            msg_p->type.ret_bool = AMTR_MGR_GetDot1dTpAgingTime(
                &msg_p->data.arg_ui32);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_ui32);
            break;

        case AMTR_MGR_IPC_SETDOT1DTPAGINGTIME:
            msg_p->type.ret_bool = AMTR_MGR_SetDot1dTpAgingTime(
                msg_p->data.arg_ui32);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;

#if (SYS_CPNT_HASH_LOOKUP_DEPTH_CONFIGURABLE == TRUE)
        case AMTR_MGR_IPC_GETHASHLOOKUPDEPTHFROMCONFIG:
            msg_p->type.ret_bool = AMTR_MGR_GetHashLookupDepthFromConfig(
                &msg_p->data.arg_ui32);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_ui32);
            break;

        case AMTR_MGR_IPC_GETHASHLOOKUPDEPTHFROMCHIP:
            msg_p->type.ret_bool = AMTR_MGR_GetHashLookupDepthFromChip(
                &msg_p->data.arg_ui32);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_ui32);
            break;

        case AMTR_MGR_IPC_SETHASHLOOKUPDEPTH:
            msg_p->type.ret_bool = AMTR_MGR_SetHashLookupDepth(
                msg_p->data.arg_ui32);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;
#endif

        case AMTR_MGR_IPC_GETDOT1DTPFDBENTRY:
            msg_p->type.ret_bool = AMTR_MGR_GetDot1dTpFdbEntry(
                &msg_p->data.arg_dot1d_tp_fdb_entry);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_dot1d_tp_fdb_entry);
            break;

        case AMTR_MGR_IPC_GETNEXTDOT1DTPFDBENTRY:
            msg_p->type.ret_bool = AMTR_MGR_GetNextDot1dTpFdbEntry(
                &msg_p->data.arg_dot1d_tp_fdb_entry);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_dot1d_tp_fdb_entry);
            break;

        case AMTR_MGR_IPC_GETDOT1DSTATICENTRY:
            msg_p->type.ret_bool = AMTR_MGR_GetDot1dStaticEntry(
                &msg_p->data.arg_dot1d_static_entry);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_dot1d_static_entry);
            break;

        case AMTR_MGR_IPC_GETNEXTDOT1DSTATICENTRY:
            msg_p->type.ret_bool = AMTR_MGR_GetNextDot1dStaticEntry(
                &msg_p->data.arg_dot1d_static_entry);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_dot1d_static_entry);
            break;

        case AMTR_MGR_IPC_SETDOT1DSTATICADDRESS:
            msg_p->type.ret_bool = AMTR_MGR_SetDot1dStaticAddress(
                msg_p->data.arg_grp_mac_mac.arg_mac_1,
                msg_p->data.arg_grp_mac_mac.arg_mac_2);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;

        case AMTR_MGR_IPC_SETDOT1DSTATICRECEIVEPORT:
            msg_p->type.ret_bool = AMTR_MGR_SetDot1dStaticReceivePort(
                msg_p->data.arg_grp_mac_ui32.arg_mac,
                msg_p->data.arg_grp_mac_ui32.arg_ui32);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;

        case AMTR_MGR_IPC_SETDOT1DSTATICALLOWEDTOGOTO:
            msg_p->type.ret_bool = AMTR_MGR_SetDot1dStaticAllowedToGoTo(
                msg_p->data.arg_grp_mac_ports.arg_mac,
                msg_p->data.arg_grp_mac_ports.arg_ports);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;

        case AMTR_MGR_IPC_SETDOT1DSTATICSTATUS:
            msg_p->type.ret_bool = AMTR_MGR_SetDot1dStaticStatus(
                msg_p->data.arg_grp_mac_ui32.arg_mac,
                msg_p->data.arg_grp_mac_ui32.arg_ui32);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;

        case AMTR_MGR_IPC_GETDOT1QFDBENTRY:
            msg_p->type.ret_bool = AMTR_MGR_GetDot1qFdbEntry(
                &msg_p->data.arg_dot1q_fdb_entry);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_dot1q_fdb_entry);
            break;

        case AMTR_MGR_IPC_GETNEXTDOT1QFDBENTRY:
            msg_p->type.ret_bool = AMTR_MGR_GetNextDot1qFdbEntry(
                &msg_p->data.arg_dot1q_fdb_entry);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_dot1q_fdb_entry);
            break;

        case AMTR_MGR_IPC_GETDOT1QTPFDBENTRY:
            msg_p->type.ret_bool = AMTR_MGR_GetDot1qTpFdbEntry(
                msg_p->data.arg_grp_ui32_dot1qtpfdb.arg_ui32,
                &msg_p->data.arg_grp_ui32_dot1qtpfdb.arg_dot1q_tp_fdb);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_dot1qtpfdb);
            break;

        case AMTR_MGR_IPC_GETNEXTDOT1QTPFDBENTRY:
            msg_p->type.ret_bool = AMTR_MGR_GetNextDot1qTpFdbEntry(
                &msg_p->data.arg_grp_ui32_dot1qtpfdb.arg_ui32,
                &msg_p->data.arg_grp_ui32_dot1qtpfdb.arg_dot1q_tp_fdb);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_dot1qtpfdb);
            break;

        case AMTR_MGR_IPC_GETDOT1QSTATICUNICASTENTRY:
            msg_p->type.ret_bool = AMTR_MGR_GetDot1qStaticUnicastEntry(
                msg_p->data.arg_grp_ui32_dot1qstaticunicast.arg_ui32,
                &msg_p->data.arg_grp_ui32_dot1qstaticunicast.arg_dot1q_static_unicast);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_dot1qstaticunicast);
            break;

        case AMTR_MGR_IPC_GETNEXTDOT1QSTATICUNICASTENTRY:
            msg_p->type.ret_bool = AMTR_MGR_GetNextDot1qStaticUnicastEntry(
                &msg_p->data.arg_grp_ui32_dot1qstaticunicast.arg_ui32,
                &msg_p->data.arg_grp_ui32_dot1qstaticunicast.arg_dot1q_static_unicast);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_dot1qstaticunicast);
            break;

        case AMTR_MGR_IPC_SETDOT1QSTATICUNICASTALLOWEDTOGOTO:
            msg_p->type.ret_bool = AMTR_MGR_SetDot1qStaticUnicastAllowedToGoTo(
                msg_p->data.arg_grp_ui32_mac_ports.arg_ui32,
                msg_p->data.arg_grp_ui32_mac_ports.arg_mac,
                msg_p->data.arg_grp_ui32_mac_ports.arg_ports);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;

        case AMTR_MGR_IPC_SETDOT1QSTATICUNICASTSTATUS:
            msg_p->type.ret_bool = AMTR_MGR_SetDot1qStaticUnicastStatus(
                msg_p->data.arg_grp_ui32_mac_ui32.arg_ui32_1,
                msg_p->data.arg_grp_ui32_mac_ui32.arg_mac,
                msg_p->data.arg_grp_ui32_mac_ui32.arg_ui32_2);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;

        case AMTR_MGR_IPC_SETLEARNINGMODE:
            msg_p->type.ret_bool = AMTR_MGR_SetLearningMode(
                msg_p->data.arg_ui32);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;

        case AMTR_MGR_IPC_ISPORTSECURITYENABLE:
            msg_p->type.ret_bool = AMTR_MGR_IsPortSecurityEnabled(
                msg_p->data.arg_ui32);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;

#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)
        case AMTR_MGR_IPC_NOTIFYINTRUSIONMAC:
            msg_p->type.ret_bool = AMTR_MGR_Notify_IntrusionMac(
                msg_p->data.arg_grp_ui32_ui16_mac_mac_ui16.arg_ui32,
                msg_p->data.arg_grp_ui32_ui16_mac_mac_ui16.arg_ui16_1,
                msg_p->data.arg_grp_ui32_ui16_mac_mac_ui16.arg_mac_1,
                msg_p->data.arg_grp_ui32_ui16_mac_mac_ui16.arg_mac_2,
                msg_p->data.arg_grp_ui32_ui16_mac_mac_ui16.arg_ui16_2);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;
#endif

        case AMTR_MGR_IPC_NOTIFYSECURITYPORTMOVE:
            AMTR_MGR_Notify_SecurityPortMove(
                msg_p->data.arg_grp_ui32_ui32_mac_ui32.arg_ui32_1,
                msg_p->data.arg_grp_ui32_ui32_mac_ui32.arg_ui32_2,
                msg_p->data.arg_grp_ui32_ui32_mac_ui32.arg_mac,
                msg_p->data.arg_grp_ui32_ui32_mac_ui32.arg_ui32_3);
            msg_p->type.ret_bool = TRUE;
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;

        case AMTR_MGR_IPC_DELETEADDRBYLIFETIMEANDVID:
            msg_p->type.ret_bool = AMTR_MGR_DeleteAddrByVidAndLifeTime(
                msg_p->data.arg_grp_ui32_alt.arg_ui32,
                msg_p->data.arg_grp_ui32_alt.arg_addresslifetime);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;

        case AMTR_MGR_IPC_SETCPUMAC:
            msg_p->type.ret_amtr_type = AMTR_MGR_SetCpuMac(
                msg_p->data.arg_grp_ui32_mac_bool.arg_ui32,
                msg_p->data.arg_grp_ui32_mac_bool.arg_mac,
                msg_p->data.arg_grp_ui32_mac_bool.arg_bool);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;

#if (SYS_CPNT_STATIC_ROUTE_AND_METER_WORKAROUND == TRUE)
        case AMTR_MGR_IPC_SETROUTERADDITIONALCTRLREG:
            msg_p->type.ret_bool = AMTR_MGR_SetRouterAdditionalCtrlReg(
                msg_p->data.arg_ui32);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;
#endif

#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
        case AMTR_MGR_IPC_SETMACNOTIFYGLOBALSTATUS:
            msg_p->type.ret_bool = AMTR_MGR_SetMacNotifyGlobalStatus(
                msg_p->data.arg_bool);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;

        case AMTR_MGR_IPC_SETMACNOTIFYINTERVAL:
            msg_p->type.ret_bool = AMTR_MGR_SetMacNotifyInterval(
                msg_p->data.arg_ui32);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;

        case AMTR_MGR_IPC_SETMACNOTIFYPORTSTATUS:
            msg_p->type.ret_bool = AMTR_MGR_SetMacNotifyPortStatus(
                msg_p->data.arg_grp_ui32_bool.arg_ui32,
                msg_p->data.arg_grp_ui32_bool.arg_bool);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;

        case AMTR_MGR_IPC_GETRUNNINGMACNOTIFYINTERVAL:
            msg_p->type.ret_running_cfg = AMTR_MGR_GetRunningMacNotifyInterval(
                &msg_p->data.arg_ui32);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_ui32);
            break;

        case AMTR_MGR_IPC_GETRUNNINGMACNOTIFYGLOBALSTATUS:
            msg_p->type.ret_running_cfg = AMTR_MGR_GetRunningMacNotifyGlobalStatus(
                &msg_p->data.arg_bool);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_bool);
            break;

        case AMTR_MGR_IPC_GETRUNNINGMACNOTIFYPORTSTATUS:
            msg_p->type.ret_running_cfg = AMTR_MGR_GetRunningMacNotifyPortStatus(
                    msg_p->data.arg_grp_ui32_bool.arg_ui32,
                    &msg_p->data.arg_grp_ui32_bool.arg_bool);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_bool);
            break;
#endif

#if (SYS_CPNT_AMTR_LOG_HASH_COLLISION_MAC == TRUE)
        case AMTR_MGR_IPC_CLEARCOLLISIONVLANMACTABLE:
            msg_p->type.ret_bool = AMTR_MGR_ClearCollisionVlanMacTable();
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;
#endif

#if (SYS_CPNT_AMTR_VLAN_MAC_LEARNING == TRUE)
        case AMTR_MGR_IPC_GETVLANLEARNINGSTATUS:
            msg_p->type.ret_bool = AMTR_MGR_GetVlanLearningStatus(
                msg_p->data.arg_grp_ui32_bool.arg_ui32,
                &msg_p->data.arg_grp_ui32_bool.arg_bool);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_bool);
            break;
        case AMTR_MGR_IPC_GETRUNNINGVLANLEARNINGSTATUS:
            msg_p->type.ret_running_cfg = AMTR_MGR_GetRunningVlanLearningStatus(
                msg_p->data.arg_grp_ui32_bool.arg_ui32,
                &msg_p->data.arg_grp_ui32_bool.arg_bool);
            msgbuf_p->msg_size = AMTR_MGR_GET_MSG_SIZE(arg_grp_ui32_bool);
            break;
        case AMTR_MGR_IPC_SETVLANLEARNINGSTATUS:
            msg_p->type.ret_bool = AMTR_MGR_SetVlanLearningStatus(
                msg_p->data.arg_grp_ui32_bool.arg_ui32,
                msg_p->data.arg_grp_ui32_bool.arg_bool);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;
#endif
#if (SYS_CPNT_MLAG == TRUE)
        case AMTR_MGR_IPC_SETMLAGMACNOTIFYPORTSTATUS:
            msg_p->type.ret_bool = AMTR_MGR_SetMlagMacNotifyPortStatus(
                msg_p->data.arg_grp_ui32_bool.arg_ui32,
                msg_p->data.arg_grp_ui32_bool.arg_bool);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;
#endif

        case AMTR_MGR_IPC_AUTHENTICATEPACKET:
            msg_p->type.ret_bool = AMTR_MGR_AuthenticatePacket(
                msg_p->data.arg_auth_pkt.src_mac,
                msg_p->data.arg_auth_pkt.vid,
                msg_p->data.arg_auth_pkt.lport,
                msg_p->data.arg_auth_pkt.auth_result);
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
            break;

        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.ret_bool = FALSE;
            msgbuf_p->msg_size = AMTR_MGR_IPCMSG_TYPE_SIZE;
    }

    AMTR_MGR_RETURN(TRUE);
} /* End of AMTR_MGR_HandleIPCReqMsg */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_AnnounceNewAddress_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: AMTRdrv announce NA to AMTR (MAC, VID, Port)
 * INPUT   : addr_buf.vid           -- which VID number
 *           addr_buf.mac           -- what's the mac address
 *           addr_buf.ifindex       -- which unit which port or which trunk_id
 *           num_of_entries         -- number of entry
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : refinement
 *           1. AMTRdrv announce NA by calling this API
 *           2. The API will call AMTR_MGR_SetAddrEetry(), and set the parameters to be
 *              addr_entry -> action     == AMTR_MGR_ACTION_FORWARD
 *              addr_entry -> source     == AMTR_MGR_SOURCE_LEARN
 *              addr_entry -> life_time  ==  AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT
 *           3. This API will receive add_buf[] from AMTRDRV's NA buffer, and check each entry.
 *              If entry passed checking, it would be set to local OM first.
 *              This is for updating AMTR counter on real time.
 *              After AMTR check every entry in addr_buf[] , AMTRDRV will send
 *              addr_buf[] to remote unit via ISC.
 *           4. num_of_entries must <= AMTRDRV_OM_MAX_ENTRIES_IN_ONE_PACKET
 *--------------------------------------------------------------------------*/
void AMTR_MGR_AnnounceNewAddress_CallBack(UI32_T num_of_entries, AMTR_TYPE_AddrEntry_T addr_buf[])
{
    Port_Info_T         p_info;     /*get port information from swctrl*/
    AMTRDRV_TYPE_Record_T get_entry;    /*get the entry from hash table of AMTRDRV */

    /* we use "SWCTRL_LogicalPortToUserPort" to check the port type of ifindex.
     * this API will return unit, port and trunk_id, so we must declare "temp_unit",
     * "temp_port" and "temp_trunk_id". These three variable just for receive the output.
     */
    SWCTRL_Lport_Type_T type;
    UI32_T              temp_unit;
    UI32_T              temp_port;
    UI32_T              temp_trunk_id;
    UI32_T              entry_index;
    /* three index for notify buffer
     */
    UI32_T              intrusion_mac_index=0;
    UI32_T              auto_learn_index=0;
    UI32_T              i;
    UI32_T              port_move_index=0;
    UI32_T              security_port_move_index=0;
    UI32_T              trunk_ifindex;
    BOOL_T              is_entry_exist_om;
    BOOL_T              is_static_trunk;
    BOOL_T              retval;
    AMTR_TYPE_Ret_T     amtr_retval;
#if (AMTR_TYPE_DEBUG_PACKET_FLOW == 1)
    if(amtr_mgr_na_interface_index)
        BACKDOOR_MGR_Printf("amtr mgr recv msg\n");
#endif
    /* BODY
     */
    if (amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN();


#ifdef AMTR_MGR_BACKDOOR_OPEN
    /* Test: AMTR takes X seconds to learn Y record
     */
    if (amtr_mgr_bd_perform_test)
    {
    /* User test "Performance Testing" procedure:
     * 1: Open Toggle by backdoor --> init amtr_mgr_bd_start_learning_tick = 0
     * 2: If (amtr_mgr_bd_start_learning_tick==0), AMTR will get the "start learning" ticks when receiving the first NA.
     * 3: If (total_counter >= amtr_mgr_bd_test_amount)
     *       printf("AMTR takes [%ld] ticks to Learn [%ld] entries")
     */
        if (amtr_mgr_bd_start_learning_tick==0)
        {
            /* Get counter if and only if receiving first NA.
             */
            if (AMTRDRV_OM_GetTotalCounter()==0)
                amtr_mgr_bd_start_learning_tick = SYSFUN_GetSysTick();
        }
    }
#endif

    for(entry_index=0; entry_index<num_of_entries;entry_index++)
    {
        is_entry_exist_om = FALSE;

        get_entry.address.vid = addr_buf[entry_index].vid;
        amtrmgr_memcpy(get_entry.address.mac, addr_buf[entry_index].mac, AMTR_TYPE_MAC_LEN);

#if (AMTR_TYPE_DEBUG_PACKET_FLOW == 1)
        if(AMTR_MGR_IS_TRACED_INTERFACE(addr_buf[entry_index].ifindex))
            BACKDOOR_MGR_Printf("%s-%d the NA is index%i, vid%i, mac %0x:%0x:%0x:%0x:%0x:%0x\r\n", __FUNCTION__, __LINE__, addr_buf[entry_index].ifindex,addr_buf[entry_index].vid,addr_buf[entry_index].mac[0],addr_buf[entry_index].mac[1],addr_buf[entry_index].mac[2],addr_buf[entry_index].mac[3],addr_buf[entry_index].mac[4],addr_buf[entry_index].mac[5]);
#endif

        /* AMTRDRV can't transform unit+port into trunk ifindex.
         * The addr_buf[entry_index].ifindex only identify ifindex of normal port.
         * In Core Layer, AMTR have to transform ifindex of trunk member into trunk_ifindex.
         * If (SWCTRL_IsTrunkMember() == TRUE), addr_buf[entry_index].ifindex will be modified to trunk ifindex.
         */
        if (SWCTRL_IsTrunkMember(addr_buf[entry_index].ifindex, &trunk_ifindex, &is_static_trunk)==TRUE)
        {
#if (AMTR_TYPE_DEBUG_PACKET_FLOW == 1)
            if(AMTR_MGR_IS_TRACED_INTERFACE(addr_buf[entry_index].ifindex))
                BACKDOOR_MGR_Printf("%s-%d the NA index is update to %lu\r\n", __FUNCTION__, __LINE__, trunk_ifindex); /*added by Jinhua Wei,because type isn't match with the variable,so change %i to %lu*/
#endif

            addr_buf[entry_index].ifindex = (UI16_T)trunk_ifindex;
        }

        /* check the entry is exist in OM or not
         */
        if(AMTRDRV_OM_GetExactRecord( (UI8_T *)&get_entry))
        {
            /* Wheather the new entry and old entry is the same or not
             */
            /* AMTR have to transform ifindex of trunk member into trunk_ifindex frist, then check ifindex.
             * Otherwise, NA which is learnt from trunk member won't be detected in OM already.
             */
#if (AMTR_TYPE_DEBUG_PACKET_FLOW == 1)
            if(AMTR_MGR_IS_TRACED_INTERFACE(addr_buf[entry_index].ifindex))
                BACKDOOR_MGR_Printf("%s-%d the NA Can be getted from the OM\r\n", __FUNCTION__, __LINE__);
#endif

            if(addr_buf[entry_index].ifindex == get_entry.address.ifindex)
            {
                /* if get_entry is learnt entry, drop NA.
                 * if get_entry is non-learnt entry, drop NA.(learnt entry only can replace learnt entry.)
                 */
#if (AMTR_TYPE_DEBUG_PACKET_FLOW == 1)
               if(AMTR_MGR_IS_TRACED_INTERFACE(addr_buf[entry_index].ifindex))
                   BACKDOOR_MGR_Printf("%s-%d the NA Can be getted from the OM and index is the same\r\n", __FUNCTION__, __LINE__);
#endif

                amtrmgr_memset(&addr_buf[entry_index],0,sizeof(AMTR_TYPE_AddrEntry_T));

                continue;
            }
            /* mac notification: Port move: step 1 - Remove */
            AMTR_MGR_MacNotifyAddNewEntry(
                get_entry.address.ifindex,
                addr_buf[entry_index].vid,
                addr_buf[entry_index].mac,
                FALSE,
                AMTR_MGR_MAC_NTFY_NONE);
            is_entry_exist_om = TRUE;
        }

        /* HW Learning these input
         */
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
    /*these checks will move to down layer  to enhance the performance, Tony.Lei */
        if ((IS_MULTICAST_MAC(addr_buf[entry_index].mac) || IS_MAC_INVALID(addr_buf[entry_index].mac)) ||
           ( IS_VID_INVALID(addr_buf[entry_index].vid))||
           (IS_IFINDEX_INVALID(addr_buf[entry_index].ifindex)))
        {
#if (AMTR_TYPE_DEBUG_PACKET_FLOW == 1)
           if(AMTR_MGR_IS_TRACED_INTERFACE(addr_buf[entry_index].ifindex))
               BACKDOOR_MGR_Printf("%s-%d the CHECK failed(mac,vid,index)\r\n", __FUNCTION__, __LINE__);
#endif

            amtrmgr_memset(&addr_buf[entry_index],0,sizeof(AMTR_TYPE_AddrEntry_T));
            continue;
        }
#endif
        type = SWCTRL_LogicalPortToUserPort(addr_buf[entry_index].ifindex, &temp_unit, &temp_port, &temp_trunk_id);
        /* check port type
         */
        if (((type != SWCTRL_LPORT_NORMAL_PORT)&&(type != SWCTRL_LPORT_TRUNK_PORT)
#if (SYS_CPNT_VXLAN == TRUE)
            &&(type != SWCTRL_LPORT_VXLAN_PORT)
#endif
            )||
            /* check vlan is exist or not
             */
            (!AMTR_MGR_CHECK_VLAN_VALIDITY(addr_buf[entry_index].vid)) ||
            /* get the port info from SWCTRL
             */
            (
#if (SYS_CPNT_VXLAN == TRUE)
                (type != SWCTRL_LPORT_VXLAN_PORT) &&
#endif
             ( (!SWCTRL_GetPortInfo(addr_buf[entry_index].ifindex, &p_info))||
                /* check port admin status and link status
                 */
                ((p_info.admin_state == VAL_ifAdminStatus_down)||(p_info.link_status == SWCTRL_LINK_DOWN))||
                /* check port spanning tree state
                 */
                (!AMTR_MGR_CHECK_STP_STATE_VALIDITY(addr_buf[entry_index].vid, addr_buf[entry_index].ifindex))
             )
            )
           )
        {
#if (AMTR_TYPE_DEBUG_PACKET_FLOW == 1)
           if(AMTR_MGR_IS_TRACED_INTERFACE(addr_buf[entry_index].ifindex))
               BACKDOOR_MGR_Printf("%s-%d the port type %i,p_info.admin_state %lu,p_info.link_status %lu\r\n", __FUNCTION__, __LINE__, type,p_info.admin_state,p_info.link_status);
           /*added by Jinhua Wei,because type isn't match with the variable,so change %i to %lu*/
#endif

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
/* Only HW Learning can delete chip directly.
 * SW Learning always maintain OM first. AMTRDRV Task will programming chip by OM.
 */
#define AMTRDRV_MGR_DeleteChipOnly(vid, mac)
#endif

            AMTRDRV_MGR_DeleteChipOnly(addr_buf[entry_index].vid, addr_buf[entry_index].mac);
            amtrmgr_memset(&addr_buf[entry_index],0,sizeof(AMTR_TYPE_AddrEntry_T));
            continue;
        }

        /* AMTR should check intrustion mac first then checking port move.
         * secure port need to check amtr_port_info.count*/
        if (AMTR_MGR_IsPortSecurityEnabled(addr_buf[entry_index].ifindex))
        {
            /* Intrusion
             */
            /* AMTR detect intrusion mac, it can't notify upper CSCs directly.
             * Because notify function is callback function. Callback function need leave semaphore.
             * If AMTR leave semaphore for notification, other threads might get semaphore and set a configure entry.
             * If user configures a vid+mac just on addr_buf[] which is already pass checking.
             * In final, Master: configure entry will replace learnt entry. ---> it's right.
             *       Slave  : learnt entry will replace configure entry. ---> it's wrong.
             * For example, 1. there are 60 entries in addr_buf[]
             *              2. 1~20 entries pass checking. entry 20 is (vid1+mac1)
             *              3. 21 entry is intrusion mac.
             *              4. AMTR_MGR release semaphore for notification.
             *              5. CLI thread get semaphore and set a config entry to AMTR_MGR.
             *              6. This configure entry (vid1+mac1) is set to local and remote unit.
             *                 (entry20 is already exist in local OM, but conifg entry can replace it.)
             *              7. After checking in Master, addr_buf[] will be sent to remote unit.
             *              8. Remote unit set every entry in addr_buf[] to it's OM and chip.
             *              9. Entry20 will replace the conifg entry in remote unit.
             */
            /* copy intrusion mac to intrusion_mac_buf[].
             * AMTR will notify intrusion after leaving semaphore.
             */
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
    /* Only HW Learning can delete chip directly.
     * SW Learning always maintain OM first. AMTRDRV Task will programming chip by OM.
     */
#define AMTRDRV_MGR_DeleteChipOnly(vid, mac)
#endif
#if (AMTR_TYPE_DEBUG_PACKET_FLOW == 1)
           if(AMTR_MGR_IS_TRACED_INTERFACE(addr_buf[entry_index].ifindex))
               BACKDOOR_MGR_Printf("%s-%d AMTR_MGR_IsPortSecurityEnabled\r\n", __FUNCTION__, __LINE__);
#endif

            AMTRDRV_MGR_DeleteChipOnly(addr_buf[entry_index].vid, addr_buf[entry_index].mac);
            amtrmgr_memcpy(intrusion_mac_buf[intrusion_mac_index].mac, addr_buf[entry_index].mac,AMTR_TYPE_MAC_LEN);
            intrusion_mac_buf[intrusion_mac_index].ifindex=addr_buf[entry_index].ifindex;
            intrusion_mac_buf[intrusion_mac_index].vid=addr_buf[entry_index].vid;
            intrusion_mac_index++;
            amtrmgr_memset(&addr_buf[entry_index],0,sizeof(AMTR_TYPE_AddrEntry_T));
            continue;
        }

        /* AMTR should check whether entry can be replaced or not, then check port move.
         * For example, old entry is (mac=A, vid=1, port=3, source=config, life_time= permanent).
         * New entry is (mac=A, vid=1, port=5, source=learnt, life_time= del_on_timeout).
         * The old entry can't be replaced by new entry.
         * If we check port move first, AMTR_MGR will notify AMTRL3 and PVLAN.
         * But in the next step, AMTR_MGR will return FALSE.
         * In final, AMTRL3 and PVLAN will think the old entry is deleted, but this entry still exist in OM.
         */
        if (is_entry_exist_om)
        {
            /* check replacement rule: NA source=learnt only can replace OM source=learnt
             *                         NA source=security can replace OM source=security and learnt
             * If (get_entry.address.source)==AMTR_TYPE_ADDRESS_SOURCE_INVALID),
             * this kind of entry won't be saved in database.
             * If design changed, AMTR have to add this checking here.
             */
            if((get_entry.address.source)!=AMTR_TYPE_ADDRESS_SOURCE_LEARN)
            {
#if (AMTR_TYPE_DEBUG_PACKET_FLOW == 1)
               if(AMTR_MGR_IS_TRACED_INTERFACE(addr_buf[entry_index].ifindex))
                   BACKDOOR_MGR_Printf("%s-%d Learn NA has been wrote by static, continue\r\n", __FUNCTION__, __LINE__);
#endif

                amtrmgr_memset(&addr_buf[entry_index],0,sizeof(AMTR_TYPE_AddrEntry_T));
                continue;
            }

            /* Procedure runs here if and only if port move.
             * AMTR already check (addr_buf[entry_index].ifindex == get_entry.address.ifindex)
             * at previous statement.
             */

            /* AMTR don't allow port in three case.
             * (1)original port is in learning mode.(have set learn_with_count)
             * (2)original port is in security mode.(port security is enabled)
             * (3)the exist entry isn't learnt entry.
             * (learnt entry only can replace learnt entry.)
             */

            /* port move case1 and case2
             */
            if (amtr_port_info[(get_entry.address.ifindex)-1].protocol!=AMTR_MGR_PROTOCOL_NORMAL)
            {
                /* copy security port move entry to buffer
                 */
#if (AMTR_TYPE_DEBUG_PACKET_FLOW == 1)
              if(AMTR_MGR_IS_TRACED_INTERFACE(addr_buf[entry_index].ifindex))
                  BACKDOOR_MGR_Printf("%s-%d Learn NA has been wrote to security_port_move_buf, continue\r\n", __FUNCTION__, __LINE__);
#endif

                amtrmgr_memcpy(security_port_move_buf[security_port_move_index].event_entry.mac, addr_buf[entry_index].mac,AMTR_TYPE_MAC_LEN);
                security_port_move_buf[security_port_move_index].event_entry.ifindex=addr_buf[entry_index].ifindex;
                security_port_move_buf[security_port_move_index].event_entry.vid=addr_buf[entry_index].vid;
                security_port_move_buf[security_port_move_index].original_port=get_entry.address.ifindex;
                security_port_move_index++;
                amtrmgr_memset(&addr_buf[entry_index],0,sizeof(AMTR_TYPE_AddrEntry_T));
                continue;
            }

            /* Record all port move mac entries for sys_callback
             */
            amtrmgr_memcpy(port_move_buf[port_move_index].event_entry.mac, addr_buf[entry_index].mac,AMTR_TYPE_MAC_LEN);
            port_move_buf[port_move_index].event_entry.ifindex=addr_buf[entry_index].ifindex;
            port_move_buf[port_move_index].event_entry.vid=addr_buf[entry_index].vid;
            port_move_buf[port_move_index].original_port=get_entry.address.ifindex;
            port_move_index++;

            /* Port move != New address replaces old entry.
             * Because every deletion have to notify AMTRL3.
             * So, port move = delete old entry + set NA.
             * But, there is exception. If port move goes across another unit.
             * (master -> slave or slave -> master)
             * Port move = delete old entry + drop NA + learn next NA
             * This is WAD.
             * Example for explain: entry1 (old)   ={mac:AA, vid:1, port:55}
             *                      entry2(NA)     ={mac:AA, vid:1, port:1}
             *                      entry3(next NA)={mac:AA, vid:1, port:1}
             * Port move = delete entry1 + drop entry2 + entry3
             * This is WAD! If port move = delete old entry + learnt new entry.
             * There is a bug. We suppose user link up port 55(entry1),
             * then move to link up port1(entry2) very soon.
             * This is port move. The entry2 should be recorded in OM.
             * But, Master's AMTR will get entry2 first because it come from Master.
             * Then receive entry1 and think port move.
             * If port move = delete exist entry + set NA.
             * entry2 will be deleted and entry1 will be set to OM.
             * So, if port move, AMTR shall delete the exist entry.
             * And, waiting for next port move entry.
             * (The next port move entry is NA, AMTR won't think it is port move.)
             * By the way, if port move on trunk, just delete ole entry.
             */


            /*EPR:NULL
             *Problem:The databases in the OM and Chip are not the same when the action
             * of port moving occurs.
             *Root Cause:when the action of port moving occurs,the  steps are
             * 1. delete the original MAC entry (both OM and chip)
             * 2. add the newer MAC entry to chip
             * 3. add the newer MAC entry to OM
             * (the base condition: when delete MAC from chip, the keyes are vid and MAC)
             * But, the function of deleting MAC will run FSM(L_HASH).the time of deleting
             * the MAC entry from chip is not stable.So when the action of deleting MAC entry
             * from chip happen after the step 2,the bug will exist.
             *Solution:
             *   Condition:
             *      1. For the MAC entry(vid,mac,interface). the Keyes must only are vid and mac
             *         both in BCM chip and OM
             *   when port movement  happens:
             *       1. Only delete the original MAC entry from the OM
             * Method:
             *     1. add a MAC entry flag to result the port-movement's entry
             *     2. when the action of deleting the entry from chip happens,pass over the step
             *       and clear the flags
             *     3. if step 2 does not happen. clear the flags when add the newer MAC entry
             *Modiy file: l_hash.c,l_hash.h,amtr_mgr.c,amtr_type.h,
             *            amtrdrv_mgr.c,amtrdrv_om.c,amtrdrv_om.h,swdrv/include/amtr_type,h,
             *            amtrdrv_mgr.h,netaccess_group.c
             *Fixed by: DanXie
             */
            AMTRDRV_MGR_DeleteAddrEntryListFromOm(&(get_entry.address));
            /*EPR:ES3628BT-FLF-ZZ-00682
             *Problem:System:DUT connect pc port cannot learn mac address after
             *        disable/enable port spanning tree.
             *Root Cause: When system happen port move on trunk,system will delete the OM
             *            (AMTRDRV_MGR_DeleteAddrEntryListFromOm)
             *            first.But The trunk speical judgement will break, it cause new MAC
             *            can not rewrite OM and CHIP!
             *Solution:   Do not check the trunk port judgement.The trunk port move active same
             *            to normal port!
             *Modify file: amtr_mgr.c
             */
#if 0 /* DanXie, Tuesday, February 10, 2009 9:58:04 */
/* AMTR support HW Learning iff standard
 */
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
            if ((addr_buf[entry_index].ifindex >= SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER)
                 || (get_entry.address.ifindex >= SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER)
                 || ((STKTPLG_POM_IFINDEX_TO_UNIT(addr_buf[entry_index].ifindex))!=
                     (STKTPLG_POM_IFINDEX_TO_UNIT(get_entry.address.ifindex))
                    )
               )
            {
                /*port move from local to remote unit (master -> slave or slave -> master)
                 */
#if (AMTR_TYPE_DEBUG_PACKET_FLOW == 1)
              if(AMTR_MGR_IS_TRACED_INTERFACE(addr_buf[entry_index].ifindex))
                  printf("delete the first packet caused by port move\n");
#endif

                amtrmgr_memset(&addr_buf[entry_index],0,sizeof(AMTR_TYPE_AddrEntry_T));
                continue;
            }
#endif
#endif /* #if 0 */
        }
#if 0  /*test memcpy cost*/
        /* check system all entry count is full ?
         */
        if (AMTRDRV_OM_GetTotalCounter() >= amtr_mgr_arl_capacity)
        {
            /* In hardware learning, AMTR can't delete entries from ASIC ARL Table
             */
            num_of_entries = entry_index;
            break;
        }
#endif
        /*check static counter*/
        /*learnt from secure port, the life time may not be del_on_timeout*/
        if ((amtr_port_info[(addr_buf[entry_index].ifindex)-1].life_time!= AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT) &&
           ((AMTRDRV_OM_GetTotalStaticCounter() >= SYS_ADPT_MAX_NBR_TOTAL_STATIC_MAC)||
            (AMTRDRV_OM_GetStaticCounterByPort(addr_buf[entry_index].ifindex) >= SYS_ADPT_MAX_NBR_OF_STATIC_MAC_PER_PORT)))
        {
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
        /* Only HW Learning can delete chip directly.
         * SW Learning always maintain OM first. AMTRDRV Task will programming chip by OM.
         */
#define AMTRDRV_MGR_DeleteChipOnly(vid, mac)
#endif
#if (AMTR_TYPE_DEBUG_PACKET_FLOW == 1)
          if(AMTR_MGR_IS_TRACED_INTERFACE(addr_buf[entry_index].ifindex))
              BACKDOOR_MGR_Printf("%s-%d the static counter is larger than max, continue\r\n", __FUNCTION__, __LINE__);
#endif

            AMTRDRV_MGR_DeleteChipOnly(addr_buf[entry_index].vid, addr_buf[entry_index].mac);
            amtrmgr_memset(&addr_buf[entry_index],0,sizeof(AMTR_TYPE_AddrEntry_T));

            if(is_entry_exist_om)
                BACKDOOR_MGR_Printf("%s-%d Debug message exist OM want continue---ERROR!!!!\r\n", __FUNCTION__,__LINE__);
            continue;
        }

        addr_buf[entry_index].source = AMTR_TYPE_ADDRESS_SOURCE_LEARN;
        addr_buf[entry_index].life_time = amtr_port_info[(addr_buf[entry_index].ifindex)-1].life_time;
        addr_buf[entry_index].action = AMTR_TYPE_ADDRESS_ACTION_FORWARDING;
        /* AMTRDRV set fail, AMTR will drop this entry.
         */
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)

        amtr_retval = AMTRDRV_MGR_SetAddrList2LocalUnit(1, &addr_buf[entry_index]);
        retval = (amtr_retval==AMTR_TYPE_RET_SUCCESS)?TRUE:FALSE;
        /* mac notification:ADD, SW Learn */
        if(retval == TRUE)
        {
            AMTR_MGR_MacNotifyAddNewEntry(
                addr_buf[entry_index].ifindex,
                addr_buf[entry_index].vid,
                addr_buf[entry_index].mac,
                TRUE,
                AMTR_MGR_MAC_NTFY_NONE);
        }
#else

        if (addr_buf[entry_index].life_time== AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)
        {
            retval = AMTRDRV_MGR_SetAddr2OMWithoutChip(1, &addr_buf[entry_index]);
            /* mac notification:ADD, HW Learn */
            if(retval == TRUE)
            {
                AMTR_MGR_MacNotifyAddNewEntry(
                    addr_buf[entry_index].ifindex,
                    addr_buf[entry_index].vid,
                    addr_buf[entry_index].mac,
                    TRUE,
                    AMTR_MGR_MAC_NTFY_NONE);
            }
        }
        else
        {
            /* life_time == permarent or del_on_reset
             */
            amtr_retval = AMTRDRV_MGR_SetAddrList2LocalUnit(1, &addr_buf[entry_index]);
            retval = (amtr_retval==AMTR_TYPE_RET_SUCCESS)?TRUE:FALSE;
        }
#endif

        if(retval == TRUE){
#if (AMTR_TYPE_DEBUG_PACKET_FLOW == 1)
              if(AMTR_MGR_IS_TRACED_INTERFACE(addr_buf[entry_index].ifindex))
                  BACKDOOR_MGR_Printf("%s-%d The NA set ok\r\n", __FUNCTION__, __LINE__);
#endif

            /* After amtrdrv_mgr set success, amtr_mgr check whether learn_with_count is full or not.
             */
            if (AMTR_MGR_IsPortSecurityEnabled(addr_buf[entry_index].ifindex))
            {
                /* If port security enabled on a port, this port is only recorded in port_security_enabled_buf[] once.
                 * Because next entry is learnt from the same port, this entry is treated as intrusion mac.
                 * And intrusion mac will be drop in previous checking.
                 */
                port_security_enabled_buf[auto_learn_index]=addr_buf[entry_index].ifindex;
                auto_learn_index++;
            }
        }else{
            /*if the addr is port move add addr fail need del it from chip*/
            if (is_entry_exist_om)
                AMTRDRV_MGR_DeleteAddrFromChip(&(get_entry.address));
        }
    }

    /* Send addr_buf[] to remote unit via ISC.
     */
    AMTRDRV_MGR_SetAddrList2RemoteUnit(num_of_entries,addr_buf);

#ifdef AMTR_MGR_BACKDOOR_OPEN
/* Performance Testing: AMTR takes X seconds to learn Y record
 */
    if(amtr_mgr_bd_perform_test)
    {
        /* 0xffff is initialize value.
         */
        if ((amtr_mgr_bd_start_learning_tick != 0 )&&(amtr_mgr_bd_start_learning_tick !=0xFFFF))
        {
                if (AMTRDRV_OM_GetTotalCounter() >= amtr_mgr_bd_test_amount)
                {
                    BACKDOOR_MGR_Printf("\r\n AMTR takes [%ld] ticks to Learn [%ld] entries ",
                               SYSFUN_GetSysTick()-amtr_mgr_bd_start_learning_tick,
                    amtr_mgr_bd_test_amount);
                    amtr_mgr_bd_start_learning_tick = 0xFFFF;
                    /* start to test the age out efficiency
                     */
                    amtr_mgr_bd_start_age_out_tick =0;
                }
        }
    }
#endif

    /* AMTR notify security port move
     */
    for (i = 0;i < security_port_move_index;i++)
    {
        AMTR_MGR_Notify_SecurityPortMove(security_port_move_buf[i].event_entry.ifindex,
                                                               security_port_move_buf[i].event_entry.vid,
                                                               security_port_move_buf[i].event_entry.mac,
                                                               security_port_move_buf[i].original_port);
    }

    /* AMTR notify learn_with_count is full. The notification have to be
     * processed earlier than intrusion.
     * Because port security is enabled first, then AMTR detect intrusion mac.
     */
    for (i = 0;i < auto_learn_index;i++)
    {
        AMTR_MGR_Notify_AutoLearnCount(port_security_enabled_buf[i], VAL_portSecPortStatus_enabled);
    }

    /* AMTR notify intrusion
     */
#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)
    for (i=0;i<intrusion_mac_index;i++)
    {
        AMTR_MGR_Notify_IntrusionMac(intrusion_mac_buf[i].ifindex,
                                     intrusion_mac_buf[i].vid,
                                     intrusion_mac_buf[i].mac,
                                     (UI8_T *)null_mac,0);
    }
#endif

    /* AMTR notify port move
     */
    if (port_move_index > 0)
    {
        AMTR_MGR_Notify_PortMove(port_move_index, port_move_buf);
    }


    AMTR_MGR_RETURN();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_AgingOut_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: AMTRdrv notify age out entries to AMTR (MAC, VID)
 * INPUT   : vid           -- which VID number
 *           *mac          -- what's the mac address
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : refinement
 *--------------------------------------------------------------------------*/
void AMTR_MGR_AgingOut_CallBack(UI32_T num_of_entries, AMTR_TYPE_AddrEntry_T addr_buf[])
{
    AMTRDRV_TYPE_Record_T  get_entry;
    UI32_T                         entry_index;

    /* BODY
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN();

#ifdef AMTR_MGR_BACKDOOR_OPEN
    if(amtr_mgr_bd_perform_test)
    {
        if (amtr_mgr_bd_start_age_out_tick == 0)
        {
                if (AMTRDRV_OM_GetTotalCounter() >= amtr_mgr_bd_test_amount)
                    amtr_mgr_bd_start_age_out_tick = SYSFUN_GetSysTick();
        }
    }
#endif
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
    AMTR_MGR_Notify_AgingOut(num_of_entries,addr_buf);
#endif

    for(entry_index=0; entry_index<num_of_entries;entry_index++)
    {
        if (IS_MAC_INVALID(addr_buf[entry_index].mac))
        {
            amtrmgr_memset(&addr_buf[entry_index],0,sizeof(AMTR_TYPE_AddrEntry_T));
            continue;
        }

        if ((addr_buf[entry_index].vid)> AMTR_TYPE_MAX_LOGICAL_VLAN_ID)
        {
            amtrmgr_memset(&addr_buf[entry_index],0,sizeof(AMTR_TYPE_AddrEntry_T));
            continue;
        }

        get_entry.address.vid = addr_buf[entry_index].vid;
        amtrmgr_memcpy(get_entry.address.mac, addr_buf[entry_index].mac, AMTR_TYPE_MAC_LEN);

        if (AMTRDRV_OM_GetExactRecord((UI8_T *)&get_entry) == FALSE)
        {
            amtrmgr_memset(&addr_buf[entry_index],0,sizeof(AMTR_TYPE_AddrEntry_T));
            continue;
        }

        /* We need to check the life time and sourec of every entry.
         * If we didn't check, there is timing issue.
         * For example, AMTR learnt a entry(vid+mac) from Savle.
         * This entry will be aged out. Salve send this age out entry via ISC.
         * Before Master receive the ISC packet, User conifure the same entry(vid+mac).
         * If Master receive this age out entry and don't check source and life time.
         * The config entry will be deleted.
         */

        /* entry will be aged out if life_time==del_on_timeout
         */
        if(get_entry.address.life_time!=AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)
        {
            amtrmgr_memset(&addr_buf[entry_index],0,sizeof(AMTR_TYPE_AddrEntry_T));
            continue;
        }

        /* If we don't filter the non-learnt entry, there are learnt entries and non-learnt
         * entries in addr_buf[]. When AMTR delete internal entries by itself, it will delete
         * wrong entries.(if and only if learnt entry make internal entry)
         */
        if((get_entry.address.source!=AMTR_TYPE_ADDRESS_SOURCE_LEARN) &&
           (get_entry.address.source!=AMTR_TYPE_ADDRESS_SOURCE_SECURITY) &&
           (get_entry.address.source!=AMTR_TYPE_ADDRESS_SOURCE_MLAG))
        {
            amtrmgr_memset(&addr_buf[entry_index],0,sizeof(AMTR_TYPE_AddrEntry_T));
            continue;
        }
        #if (AMTR_TYPE_DEBUG_PACKET_FLOW == 1)
            if(AMTR_MGR_IS_TRACED_INTERFACE(addr_buf[entry_index].ifindex))
                BACKDOOR_MGR_Printf("%s-%d the Age-Out is index%i, vid%i, mac %0x:%0x:%0x:%0x:%0x:%0x, index:%d\r\n", __FUNCTION__, __LINE__, addr_buf[entry_index].ifindex,addr_buf[entry_index].vid,addr_buf[entry_index].mac[0],addr_buf[entry_index].mac[1],addr_buf[entry_index].mac[2],addr_buf[entry_index].mac[3],addr_buf[entry_index].mac[4],addr_buf[entry_index].mac[5], get_entry.address.ifindex);
        #endif
        /* copy complete entry from get_entry to addr_buf[entry_index]
         */
        amtrmgr_memcpy(&addr_buf[entry_index], &get_entry.address, sizeof(AMTR_TYPE_AddrEntry_T));
        /* mac notification: REMOVE  get_entry.address.ifindex */
        AMTR_MGR_MacNotifyAddNewEntry(
            get_entry.address.ifindex,
            get_entry.address.vid,
            get_entry.address.mac,
            FALSE,
            AMTR_MGR_MAC_NTFY_NONE);
    }
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
    AMTRDRV_MGR_DeleteAddrEntryList(num_of_entries, addr_buf);
#else
    /* In HW learning, only update OM, doesn't need to delete ASIC ARL Table.
     */
    AMTRDRV_MGR_DeleteAddrFromOMWithoutChip(num_of_entries, addr_buf);
#endif

#ifdef AMTR_MGR_BACKDOOR_OPEN
    /* Performance Testing: AMTR takes X seconds to learn Y record
     */
    if(amtr_mgr_bd_perform_test)
    {
        if (amtr_mgr_bd_start_age_out_tick!= 0 )
        {
                if (AMTRDRV_OM_GetTotalCounter() ==0)
                {
                    BACKDOOR_MGR_Printf("\r\n AMTR takes [%ld] ticks to Age Out [%ld] entries ",
                               SYSFUN_GetSysTick()-amtr_mgr_bd_start_age_out_tick,
                         amtr_mgr_bd_test_amount);
                }
        }
    }
#endif
    AMTR_MGR_RETURN();
}

#if 0 /* obsoleted API */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_SecurityCheck_Callback
 * -------------------------------------------------------------------------
 * FUNCTION: This API will check intrusion and port move.
 * INPUT   : vid           -- which VID number
 *           *mac          -- what's the mac address
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : refinement
 *           1.When lan.c receive packet, AMTR have to check whether it's intrusion or not.
 *             Intrusion mac can't be put in NA buffer and can't run procedure about protocol.
 *           2. In Hardware Learning, AMTR notify intrusion mac by this callback function.
 *--------------------------------------------------------------------------*/
UI32_T AMTR_MGR_SecurityCheck_Callback( UI32_T src_lport, UI16_T vid, UI8_T *src_mac, UI8_T *dst_mac, UI16_T ether_type)
{
    UI32_T          trunk_ifindex;
    BOOL_T          is_static_trunk;
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
    AMTRDRV_TYPE_Record_T  original_entry;
    BOOL_T is_exist_om = FALSE;

    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        return AMTR_TYPE_CHECK_INTRUSION_RETVAL_LEARN;

    /* AMTRDRV can't transform unit+port into trunk ifindex.
     * The addr_buf[entry_index].ifindex only identify ifindex of normal port.
     * In Core Layer, AMTR have to transform ifindex of trunk member into trunk_ifindex.
     * If (SWCTRL_IsTrunkMember() == TRUE), src_lport will be modified to trunk ifindex.
     */
    if (SWCTRL_IsTrunkMember(src_lport, &trunk_ifindex, &is_static_trunk)==TRUE)
    {
        src_lport = trunk_ifindex;
    }

    if(AMTR_MGR_IsPortSecurityEnabled(src_lport))
    {
        /* learn_with_count is full, no learn.
         */
#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)
        if (AMTR_MGR_Notify_IntrusionMac(src_lport, vid, src_mac,dst_mac, ether_type))
            return AMTR_TYPE_CHECK_INTRUSION_RETVAL_DROP;   /*no learnt & drop*/
        else
#endif
            return 0;                                       /*no learnt & no drop*/
    }

    original_entry.address.vid=vid;
    amtrmgr_memcpy(original_entry.address.mac,src_mac,AMTR_TYPE_MAC_LEN);
    is_exist_om = AMTRDRV_OM_GetExactRecord((UI8_T *)&original_entry);


    /* already exist in OM.
     * There are three conditions, the kind of packets will be trapped to CPU.
     * 1. DA=CPU MAC; 2. This entry exist in OM, but doesn't in chip yet. 3. port move on chip
     */
    if (is_exist_om)
    {
        /* 1. already exist in OM but not in ASIC ARL Table. This entry is in job queue
         *    and wait to programming chip. So, no learn and no drop.
         * 2. If DA of IP packet is CPU MAC, it's SA will be changed to CPU MAC too.
         *    This is a erratum on XGS. So, AMTR must workaround here.
         *    If NA is CPU MAC, no learn and no drop.
         */
        if ((original_entry.address.ifindex==src_lport)||(original_entry.address.source==AMTR_TYPE_ADDRESS_SOURCE_SELF))
            return 0;/* no learnt & no drop*/
        else /*port move*/
        {
            /* port move from secure port, drop & no learn
             */
            if (amtr_port_info[(original_entry.address.ifindex)-1].protocol!=AMTR_MGR_PROTOCOL_NORMAL)
            {
                AMTR_MGR_Notify_SecurityPortMove(src_lport, original_entry.address.vid, src_mac, original_entry.address.ifindex);
                return AMTR_TYPE_CHECK_INTRUSION_RETVAL_DROP;  /*no learnt & drop*/
            }

            /* port move from normal, check replacement rule.
             * If (get_entry.address.source)==AMTR_TYPE_ADDRESS_SOURCE_INVALID),
             * this kind of entry won't be saved in database.
             * If design changed, AMTR have to add this checking here.
             */
            if (original_entry.address.source!=AMTR_TYPE_ADDRESS_SOURCE_LEARN)
                return AMTR_TYPE_CHECK_INTRUSION_RETVAL_DROP;  /*no learnt & drop*/
        }
    }

    return AMTR_TYPE_CHECK_INTRUSION_RETVAL_LEARN; /*learnt & no drop*/

#else /* SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE*/

    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        return AMTR_TYPE_CHECK_INTRUSION_RETVAL_LEARN;

    if (SWCTRL_IsTrunkMember(src_lport, &trunk_ifindex, &is_static_trunk)==TRUE)
    {
        src_lport = trunk_ifindex;
    }

    /* In HW learning, AMTR notify to PSec if and only if intruction mac.
     * (not ask PSec to do intrusion checking)
     * So, DA = NULL MAC; ether type = 0.
     */
#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)
    if (AMTR_MGR_Notify_IntrusionMac(src_lport, vid, src_mac,(UI8_T *)null_mac,0))
        return AMTR_TYPE_CHECK_INTRUSION_RETVAL_DROP;   /*no learnt & drop*/
    else
#endif
        return 0;                                       /*no learnt & no drop*/
#endif
}
#endif /* end of #if 0 obsoleted API */

void AMTR_MGR_PortLinkDown_CallBack(UI32_T l_port)
{
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        return;

    /* handle mac-notify in AMTR_OM_HisamDeleteByPortNLifeTimeCallBack()
     */
    AMTRDRV_MGR_DeleteAddrByPortAndLifeTime(l_port,AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);
    AMTR_MGR_Notify_MACTableDeleteByPort(l_port, AMTR_MGR_UPORT_DOWN);
    return;
}

void AMTR_MGR_PortAdminDisable_CallBack(UI32_T l_port)
{
    AMTRDRV_MGR_DeleteAddrByPortAndLifeTime(l_port,AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);
    AMTR_MGR_Notify_MACTableDeleteByPort(l_port, AMTR_MGR_ADMIN_DISABLE);
    return;
}

void AMTR_MGR_PortLearningStatusChanged_Callback(UI32_T l_port, BOOL_T learning)
{
    if (!learning)
    {
        AMTRDRV_MGR_DeleteAddrByPortAndLifeTime(l_port,AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);
        AMTR_MGR_Notify_MACTableDeleteByPort(l_port, AMTR_MGR_LEARNING_DISABLE);
    }
}

void AMTR_MGR_PortAddIntoTrunk_CallBack(UI32_T l_port_trunk, UI32_T l_port_member,BOOL_T is_firstmem)
{
    UI32_T              unit;
    UI32_T              u_port;
    UI32_T              trunk_id;
    SWCTRL_Lport_Type_T type;
    BOOL_T              retval;
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN();
    if(l_port_member < 1 || l_port_member > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
        AMTR_MGR_RETURN();

    type = SWCTRL_LogicalPortToUserPort(l_port_member, &unit, &u_port, &trunk_id);
    if(type != SWCTRL_LPORT_TRUNK_PORT_MEMBER)
        AMTR_MGR_RETURN();
    /*add by tony.lei*/
   if(is_firstmem == FALSE){

       if(amtr_port_info[l_port_trunk-1].is_learn != amtr_port_info[l_port_member-1].is_learn){
           if(amtr_port_info[l_port_trunk-1].is_learn == TRUE)
              {
               AMTR_MGR_SetMACLearningStatusOnPort(unit,u_port,AMTR_MGR_ENABLE_MACLEARNING_ONPORT);
              }
           else
               AMTR_MGR_SetMACLearningStatusOnPort(unit,u_port,AMTR_MGR_DISABLE_MACLEARNING_ONPORT);

        }
    }else
       amtr_port_info[l_port_trunk-1].is_learn = amtr_port_info[l_port_member-1].is_learn;

    retval=AMTRDRV_MGR_DeleteAddrByPort(l_port_member);
    AMTR_MGR_Notify_MACTableDeleteByPort(l_port_member, AMTR_MGR_TRUNK_MEMBER_ADD);
}

void AMTR_MGR_DestroyTrunk_CallBack(UI32_T l_port_trunk, UI32_T l_port_member)
{
    UI32_T      unit;
    UI32_T      u_port;
    UI32_T      trunk_id;
    SWCTRL_Lport_Type_T l_port_type;
    BOOL_T              retval;

    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN();

    if(l_port_member <1 || l_port_member > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
        AMTR_MGR_RETURN();

    if(l_port_trunk <1 || l_port_trunk > SYS_ADPT_TOTAL_NBR_OF_LPORT)
        AMTR_MGR_RETURN();

    l_port_type = SWCTRL_LogicalPortToUserPort(l_port_trunk, &unit, &u_port, &trunk_id);

    if (l_port_type != SWCTRL_LPORT_TRUNK_PORT)
       {
        /* EPR:ES4827G-FLF-ZZ-00328
               *Problem: It's a system design issue
               *RootCause: database cleaning up send asynchronism message,lead to show this print sentence.
               *Solution:template solution,work around
               *File:Amtr_mgr.c
               *approve:Hard.Sun
               *Fixed by Jinhua.Wei
               */
#if 0 /* JinhuaWei, 11 August, 2008 4:20:35 */
        SYSFUN_LogMsg("Invalid trunk port: %d\r\n", l_port_trunk, 0, 0, 0, 0, 0);
#endif /* #if 0 */
       }

    retval=AMTRDRV_MGR_DeleteAddrByPort(l_port_trunk);

    AMTR_MGR_Notify_MACTableDeleteByPort(l_port_trunk, AMTR_MGR_TRUNK_DESTORY);
    return;
}

void AMTR_MGR_DestroyVlan_CallBack(UI32_T vid_ifidx,  UI32_T vlan_status)
{
    UI32_T vid;
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN();

    if(VLAN_IFINDEX_CONVERTTO_VID(vid_ifidx, vid)==0)
        AMTR_MGR_RETURN();

    AMTRDRV_MGR_DeleteAddrByVID(vid);
    AMTR_MGR_Notify_MACTableDeleteByVid(vid);
}

void AMTR_MGR_VlanMemberDelete_CallBack(UI32_T vid_ifidx, UI32_T l_port, UI32_T vlan_status)
{
    UI32_T  unit;
    UI32_T  vid;
    UI32_T  u_port;
    UI32_T  trunk_id;
    UI32_T  action_count=0; /*How many entries are deleted in Hisam*/

    SWCTRL_Lport_Type_T port_type;

    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN();

    if(VLAN_IFINDEX_CONVERTTO_VID(vid_ifidx, vid)==0)
        AMTR_MGR_RETURN();

    if(FALSE == XSTP_OM_IsPortForwardingStateByVlan(vid, l_port))
    {
        AMTR_MGR_RETURN();
    }

    port_type = SWCTRL_LogicalPortToUserPort(l_port, &unit, &u_port, &trunk_id);
    if ((port_type != SWCTRL_LPORT_NORMAL_PORT)&&(port_type != SWCTRL_LPORT_TRUNK_PORT))
        AMTR_MGR_RETURN();

    if( AMTRDRV_MGR_DeleteAddrByPortAndVidAndLifeTime(l_port,vid,AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT, FALSE)==TRUE)
    {
        /* del_by_port+vid+life_time, AMTRDRV won't sync to AMTR_MGR.
         * Core layer have to update hisam table by itself.
         */
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
        BOOL_T do_again;
        do {
#endif
        AMTR_OM_HisamDeleteRecordByLPortAndVidAndLifeTime(l_port,vid, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT,&action_count);
        AMTR_MGR_UpdatePortSecurityStatus(l_port);
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
        AMTR_MGR_NOTIFY_EDIT_ADDR_ENTRY_FROM_BUFFER(action_count, TRUE, &do_again);
        } while (do_again);
#endif
    }

    AMTR_MGR_RETURN();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_AuthenticatePacket_Callback
 * -------------------------------------------------------------------------
 * FUNCTION: This API will handle the packet by authenticated result
 * INPUT   : src_mac
 *           tag_info
 *           unit
 *           port
 *           auth_result
 *           cookie
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *--------------------------------------------------------------------------*/
void AMTR_MGR_AuthenticatePacket_Callback(
    UI8_T *src_mac,
    UI16_T tag_info,
    UI32_T unit,
    UI32_T port,
    SYS_CALLBACK_MGR_AuthenticatePacket_Result_T auth_result,
    void *cookie)
{
    AMTR_TYPE_AddrEntry_T addr_entry;
    UI32_T ifindex;

    /* default behavior:
     *   learn with life_time == DELETE_ON_TIMEOUT and
     *   announce to upper layer
     */
    BOOL_T life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT;
    BOOL_T to_learn = TRUE;
    BOOL_T to_drop = FALSE;
    BOOL_T is_security_mac = FALSE;

    if (SWCTRL_UserPortToLogicalPort(unit, port, &ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        SYS_CALLBACK_MGR_AuthenticatePacket_AsyncReturn(SYS_MODULE_AMTR, SYS_CALLBACK_MGR_AUTH_FAILED, cookie);
        return;
    }

    /* get current learning status
     */
    {
        UI32_T learning_disabled_status, intruder_handlers;
        BOOL_T vlan_learn_status = TRUE;

        if (!SWCTRL_GetPortLearningStatusEx(ifindex, &learning_disabled_status, &intruder_handlers))
        {
            SYS_CALLBACK_MGR_AuthenticatePacket_AsyncReturn(SYS_MODULE_AMTR, SYS_CALLBACK_MGR_AUTH_FAILED, cookie);
            return;
        }
        to_learn = !learning_disabled_status;

    #if (SYS_CPNT_AMTR_VLAN_MAC_LEARNING == TRUE)
        if (!AMTR_OM_GetVlanLearningStatus(tag_info&0xFFF, &vlan_learn_status))
        {
            SYS_CALLBACK_MGR_AuthenticatePacket_AsyncReturn(SYS_MODULE_AMTR, SYS_CALLBACK_MGR_AUTH_FAILED, cookie);
            return;
        }
        to_learn &= vlan_learn_status;
    #endif
    }

    /* default behavior:
     *   learn with life_time == DELETE_ON_TIMEOUT and
     *   announce to upper layer
     */
    switch (auth_result)
    {
        /* result authenticated has not determined,
         * don't learn this mac now.
         */
        case SYS_CALLBACK_MGR_AUTH_PENDING:
            to_learn = FALSE;
            break;

        /* for authenticated packet,
         * learn with life_time == DELETE_ON_RESET
         */
        case SYS_CALLBACK_MGR_AUTH_AUTHENTICATED:
            life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET;
            is_security_mac = TRUE;
            break;

        /* for authenticated packet,
         * learn with life_time == PERMANENT
         */
        case SYS_CALLBACK_MGR_AUTH_AUTHENTICATED_PERMANENTLY:
            life_time = AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT;
            is_security_mac = TRUE;
            break;

        /* for unauthenticated packet,
         * don't learn and discard this packet.
         */
        case SYS_CALLBACK_MGR_AUTH_UNAUTHENTICATED:
        case SYS_CALLBACK_MGR_AUTH_FAILED:
            to_learn = FALSE;
            to_drop = TRUE;
            break;

        /* use default behavior
         */
        case SYS_CALLBACK_MGR_AUTH_AUTHENTICATED_TEMPORARILY:
            is_security_mac=TRUE;
        case SYS_CALLBACK_MGR_AUTH_BYPASS:
        default:
            ;
    }

#if (SYS_CPNT_VRRP == TRUE)
    {
#define IS_VRRP_GROUP_MAC(mac) (((mac)[0]==0x00)&&((mac)[1]==0x00)&&((mac)[2]==0x5e)&& \
                               ((mac)[3]==0x00)&&((mac)[4]==0x01))

        if(IS_VRRP_GROUP_MAC(src_mac))
        {
            AMTR_TYPE_AddrEntry_T vmac_entry;

            /* When VRRP master receive advertisement,
             * the received vrrp packet will cause port move,
             * because VRRP master already add the same virtaul mac address as static mac.
             * The new learn virtual mac can't replace the self static mac,
             * we should bypass learning process and annouce this packet to VRRP.
             */
            memset(&vmac_entry, 0, sizeof(vmac_entry));
            vmac_entry.vid = tag_info & 0xfff;
            memcpy(vmac_entry.mac, src_mac, sizeof(vmac_entry.mac));
            if((TRUE == AMTR_MGR_GetExactAddrEntry(&vmac_entry))&&
               (vmac_entry.source == AMTR_TYPE_ADDRESS_SOURCE_SELF))
            {
                to_learn = FALSE;
            }
        }
    }
#endif

    if (to_learn)
    {
        memset(&addr_entry, 0, sizeof(addr_entry));
        memcpy(addr_entry.mac, src_mac, sizeof(addr_entry.mac));
        addr_entry.vid = tag_info & 0xfff;
        addr_entry.ifindex = ifindex;
        addr_entry.action = AMTR_TYPE_ADDRESS_ACTION_FORWARDING;
        addr_entry.source = (is_security_mac==TRUE)?AMTR_TYPE_ADDRESS_SOURCE_SECURITY:AMTR_TYPE_ADDRESS_SOURCE_LEARN;
        addr_entry.life_time = life_time;

        if (AMTR_MGR_SetAddrEntry(&addr_entry)==FALSE)
        {
            /* EPR:ECS4210-52P-00264
             * HeadLine:PortSecurity: After DUT learned 1024 security MAC,
             *          DUT will transmit multicast of the 1025th SA MAC.
             * Description: When the mac address cannot be set, the mac
             *              should be considered as authentication failed
             *              and drop it.
             */
            to_drop=TRUE;
        }
    }

    if (to_drop)
    {
        SYS_CALLBACK_MGR_AuthenticatePacket_AsyncReturn(SYS_MODULE_AMTR, SYS_CALLBACK_MGR_AUTH_FAILED, cookie);
    }
    else
    {
        SYS_CALLBACK_MGR_AuthenticatePacket_AsyncReturn(SYS_MODULE_AMTR, SYS_CALLBACK_MGR_AUTH_BYPASS, cookie);
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_SyncHashToHisam
 * -------------------------------------------------------------------------
 * FUNCTION: this function will sync entry(add or delete) from Hash table to Hisam talbe
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : sync counter
 * NOTE    : refinement     ;water_huang add (93.8.16)
 *          1.Amtr_task will call this API in periodic, then sync entry or action
 *            frome Hash table to Hisam talbe.
 *--------------------------------------------------------------------------*/
int AMTR_MGR_SyncHashToHisam(void)
{
    /* the entry in sync-queue will be added or deleted from Hisam
     */
    AMTR_TYPE_AddrEntry_T addr_entry;
    AMTR_TYPE_Command_T action;
    /* AMTR synchronize one entry to Hisam, this counter++.
     * If AMTR synchronize del_by_group, this counter +=group_sync_counter
     */
    UI32_T  total_sync_counter = 0;
    UI32_T  group_sync_counter = 0;
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
    BOOL_T  to_delete;
    BOOL_T  do_again;
#endif

    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(0);

    while(total_sync_counter < AMTR_TYPE_SYNC2HISAM_NUM){
        if(!AMTRDRV_MGR_SyncToHisam(&addr_entry, &action))
            break;                          /*sync-queue is null*/
        else{
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
            /* wakka:
             * Notify_EditAddrEntry not support AMTR_TYPE_COMMAND_DELETE_ALL now
             */
            to_delete = (action != AMTR_TYPE_COMMAND_SET_ENTRY);
            group_sync_counter = 0;

#if(SYS_CPNT_VXLAN == TRUE)
            if(AMTR_TYPE_LOGICAL_VID_IS_L_VFI_ID(addr_entry.vid))
            {
                UI16_T r_vfi;

                AMTRDRV_OM_ConvertLvfiToRvfi(addr_entry.vid, &r_vfi);
                addr_entry.vid = r_vfi;
            }
#endif

            do {
#endif
            switch (action){
                case AMTR_TYPE_COMMAND_SET_ENTRY :
                    AMTR_OM_HisamSetRecord(&addr_entry);
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
                    AMTR_MGR_Notify_EditAddrEntry(addr_entry.vid, addr_entry.mac, addr_entry.ifindex, FALSE);
                    AMTR_MGR_NOTIFY_MLAG_ADDR_ENTRY(&addr_entry, FALSE);
#endif
                    total_sync_counter++;
                    break;
                case AMTR_TYPE_COMMAND_DELETE_ENTRY :
                    AMTR_OM_HisamDeleteRecord(addr_entry.vid, addr_entry.mac);
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
                    AMTR_MGR_Notify_EditAddrEntry(addr_entry.vid, addr_entry.mac, addr_entry.ifindex, TRUE);
                    AMTR_MGR_NOTIFY_MLAG_ADDR_ENTRY(&addr_entry, FALSE);
#endif
                    total_sync_counter++;
                    break;
                case AMTR_TYPE_COMMAND_DELETE_ALL :
                    AMTR_OM_HisamDeleteAllRecord();
                    total_sync_counter+=AMTR_TYPE_SYNC2HISAM_NUM;
                    break;
                case AMTR_TYPE_COMMAND_DELETE_BY_LIFETIME :
                    AMTR_OM_HisamDeleteRecordByLifeTime(addr_entry.life_time,&group_sync_counter);
                    total_sync_counter += group_sync_counter;
                    break;
                case AMTR_TYPE_COMMAND_DELETE_BY_SOURCE :
                    AMTR_OM_HisamDeleteRecordBySource(addr_entry.source,&group_sync_counter);
                    total_sync_counter += group_sync_counter;
                    break;
                case AMTR_TYPE_COMMAND_DELETE_BY_PORT :
                    AMTR_OM_HisamDeleteRecordByLPort(addr_entry.ifindex,&group_sync_counter);
                    total_sync_counter += group_sync_counter;
                    break;
                case AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_LIFE_TIME :
                    AMTR_OM_HisamDeleteRecordByLPortAndLifeTime(addr_entry.ifindex,addr_entry.life_time,&group_sync_counter);
                    total_sync_counter += group_sync_counter;
                    break;
                case AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_SOURCE :
                    AMTR_OM_HisamDeleteRecordByLPortAndSource(addr_entry.ifindex,addr_entry.source,&group_sync_counter);
                    total_sync_counter += group_sync_counter;
                    break;
                case AMTR_TYPE_COMMAND_DELETE_BY_VID :
                    AMTR_OM_HisamDeleteRecordByVid(addr_entry.vid, &group_sync_counter);
                    total_sync_counter += group_sync_counter;
                    break;
                case AMTR_TYPE_COMMAND_DELETE_BY_VID_N_LIFETIME :
                    AMTR_OM_HisamDeleteRecordByVidAndLifeTime(addr_entry.vid,addr_entry.life_time,&group_sync_counter);
                    total_sync_counter += group_sync_counter;
                    break;
                case AMTR_TYPE_COMMAND_DELETE_BY_VID_N_SOURCE :
                    AMTR_OM_HisamDeleteRecordByVidAndSource(addr_entry.vid,addr_entry.source,&group_sync_counter);
                    total_sync_counter += group_sync_counter;
                    break;
                case AMTR_TYPE_COMMAND_DELETE_BY_VID_N_PORT :
                    AMTR_OM_HisamDeleteRecordByLPortAndVid(addr_entry.ifindex,addr_entry.vid,&group_sync_counter);
                    total_sync_counter += group_sync_counter;
                    break;
                case AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_VID_N_LIFETIME :
                    AMTR_OM_HisamDeleteRecordByLPortAndVidAndLifeTime(addr_entry.ifindex, addr_entry.vid, addr_entry.life_time, &group_sync_counter);
                    total_sync_counter += group_sync_counter;
                    break;
                case AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_VID_N_SOURCE :
                    AMTR_OM_HisamDeleteRecordByLPortAndVidAndSource(addr_entry.ifindex, addr_entry.vid, addr_entry.source, &group_sync_counter);
                    total_sync_counter += group_sync_counter;
                    break;
                case AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_VID_EXCEPT_CERTAIN_ADDR :
                    /* for this function, AMTRDRV won't sync to AMTR, so do nothing here.
                     */
                    break;
                default:
                    break;
            }

#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY == TRUE)
            AMTR_MGR_NOTIFY_EDIT_ADDR_ENTRY_FROM_BUFFER(group_sync_counter, to_delete, &do_again);
            } while (do_again);
#endif
#ifdef AMTR_MGR_BACKDOOR_OPEN
            if(amtr_mgr_bd_print_msg)
                BACKDOOR_MGR_Printf("AMTR_MGR_SyncHashToHisam()\r\n");
#endif
        }
    }

    if (total_sync_counter)
    {
        AMTR_MGR_UpdatePortSecurityStatus(0);
    }

    AMTR_MGR_RETURN(total_sync_counter);
}

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_SetCpuMac
 *------------------------------------------------------------------------------
 * Purpose  : This routine will add a CPU MAC to the chip.  Packets with this
 *            MAC in the specified VLAN will trap to CPU.
 * INPUT    : vid        -- vlan ID
 *            mac        -- MAC address
 *            is_routing -- whether the routing feature is supported and enabled
 * OUTPUT   : None
 * RETURN   : AMTR_TYPE_Ret_T     Success, or cause of fail.
 * NOTE     : None
 *------------------------------------------------------------------------------*/
AMTR_TYPE_Ret_T  AMTR_MGR_SetCpuMac(UI32_T vid, UI8_T *mac, BOOL_T is_routing)
{
    AMTR_TYPE_Ret_T ret;
    UI32_T l3_ret;

    ret = AMTR_MGR_SetInterventionEntry(vid, mac);
    if (AMTR_TYPE_RET_SUCCESS == ret)
    {
#if (SYS_CPNT_ROUTING == TRUE) && (SYS_CPNT_SWDRVL3 == TRUE)
        if (is_routing)
            l3_ret = SWDRVL3_SetL3Bit(mac, vid);
        else
            l3_ret = SWDRVL3_UnSetL3Bit(mac, vid);

        if(l3_ret != SWDRVL3_L3_NO_ERROR)
            ret = AMTR_TYPE_RET_L3_FAIL;
#endif
    }
    return ret;
}

#if (SYS_CPNT_STATIC_ROUTE_AND_METER_WORKAROUND == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_SetRouterAdditionalCtrlReg
 *------------------------------------------------------------------------------
 * Purpose  : For enable static  route or meter
 * INPUT    : value
 *                      True -- Set RouterAdditionalCtrlReg, meter work
 *                      False -- unSet RouterAdditionalCtrlReg, static route work
 * OUTPUT   : None
 * RETURN   : True : successs, False : failed
 * NOTE     : None
 *------------------------------------------------------------------------------*/
 BOOL_T  AMTR_MGR_SetRouterAdditionalCtrlReg(UI32_T value)
{
    return SWDRVL3_SetRouterAdditionalCtrlReg(value);
}
#endif /*#if (SYS_CPNT_STATIC_ROUTE_AND_METER_WORKAROUND == TRUE)*/

#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_MacNotifyProcessQueue
 *------------------------------------------------------------------------------
 * Purpose  : To process the mac-notification entries in the queue
 * INPUT    : cur_time_stamp - current time stamp in ticks
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
void  AMTR_MGR_MacNotifyProcessQueue(UI32_T  cur_time_stamp)
{
    AMTR_MGR_MacNotifyRec_T  *rec_p;
    UI32_T  time_passed, cur_pos, ntfy_cnt = 0, used_head = 0, idx, mac_ntfy_time_stamp = 0;
    UI32_T  mac_ntfy_interval = SYS_DFLT_AMTR_MAC_NOTIFY_TRAP_INTERVAL*SYS_BLD_TICKS_PER_SECOND;
    BOOL_T  is_enabled = FALSE, is_send = FALSE;

    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN();

    if (TRUE == AMTR_OM_GetMacNotifyGlobalStatus(&is_enabled) && TRUE == is_enabled)
    {
        AMTR_OM_GetMacNotifyTimeStamp(&mac_ntfy_time_stamp);
        if (mac_ntfy_time_stamp > cur_time_stamp)
        {
            time_passed = (0xFFFFFFFF - mac_ntfy_time_stamp) + cur_time_stamp;
        }
        else
        {
            time_passed = cur_time_stamp - mac_ntfy_time_stamp;
        }

        AMTR_OM_GetMacNotifyInterval(&mac_ntfy_interval);
        is_send = (time_passed >= mac_ntfy_interval) ? TRUE : FALSE;
    }

    if (TRUE == is_send)
    {
        AMTR_OM_MacNotifyCopyAndClearQueue(mac_ntfy_rec_lst, &ntfy_cnt, &used_head);

        for (idx = 0; idx < ntfy_cnt; idx ++)
        {
            cur_pos = (used_head+idx) % AMTR_MGR_MAC_NTFY_LST_MAX;
            rec_p = &mac_ntfy_rec_lst[cur_pos];

            AMTR_MGR_MacNotifySendTrap(
                rec_p->ifidx,
                rec_p->act_vid & AMTR_MGR_MAC_NTFY_VID_MASK,
                rec_p->src_mac,
                (rec_p->act_vid & AMTR_MGR_MAC_NTFY_ACT_MASK) ?
                TRUE : FALSE);
        }

        if(amtr_mgr_bd_print_msg && ntfy_cnt != 0)
            BACKDOOR_MGR_Printf("%s:%d: head/ntfy_cnt-%ld/%ld\r\n", __FUNCTION__, __LINE__, used_head, ntfy_cnt);
    }

    return;
}
/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetMacNotifyGlobalStatus
 *------------------------------------------------------------------------
 * FUNCTION: To get the mac-notification-trap global status
 * INPUT   : None
 * OUTPUT  : *is_enabled_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -----------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetMacNotifyGlobalStatus(BOOL_T *is_enabled_p)
{
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    return AMTR_OM_GetMacNotifyGlobalStatus(is_enabled_p);
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_SetMacNotifyGlobalStatus
 *------------------------------------------------------------------------
 * FUNCTION: To set the mac-notification-trap global status
 * INPUT   : None
 * OUTPUT  : is_enabled - global status to set
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -----------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_SetMacNotifyGlobalStatus(BOOL_T is_enabled)
{
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    return AMTR_OM_SetMacNotifyGlobalStatus(is_enabled);
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetMacNotifyInterval
 *------------------------------------------------------------------------
 * FUNCTION: To get the mac-notification-trap interval
 * INPUT   : None
 * OUTPUT  : interval_p - pointer to interval to get (in seconds)
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -----------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetMacNotifyInterval(UI32_T  *interval_p)
{
    BOOL_T  ret;

    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    ret = AMTR_OM_GetMacNotifyInterval(interval_p);
    if (TRUE == ret)
    {
        *interval_p /= SYS_BLD_TICKS_PER_SECOND;
    }

    return ret;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_SetMacNotifyInterval
 *------------------------------------------------------------------------
 * FUNCTION: To set the mac-notification-trap interval
 * INPUT   : None
 * OUTPUT  : interval - interval to set (in seconds)
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -----------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_SetMacNotifyInterval(UI32_T  interval)
{
    BOOL_T  ret;

    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    ret = AMTR_OM_SetMacNotifyInterval(interval * SYS_BLD_TICKS_PER_SECOND);

    return ret;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetMacNotifyPortStatus
 *------------------------------------------------------------------------
 * FUNCTION: To get the mac-notification-trap port status
 * INPUT   : ifidx        - lport ifindex
 * OUTPUT  : *is_enabled_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -----------------------------------------------------------------------
 */
BOOL_T  AMTR_MGR_GetMacNotifyPortStatus(UI32_T  ifidx, BOOL_T *is_enabled_p)
{
    BOOL_T  ret = FALSE;

    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if(SWCTRL_LogicalPortExisting(ifidx))
    {
        ret = AMTR_OM_GetMacNotifyPortStatus(ifidx, is_enabled_p);
    }

    return ret;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_SetMacNotifyPortStatus
 *------------------------------------------------------------------------
 * FUNCTION: To set the mac-notification-trap port status
 * INPUT   : ifidx      - lport ifindex
 * OUTPUT  : is_enabled - port status to set
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -----------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_SetMacNotifyPortStatus(UI32_T  ifidx, BOOL_T  is_enabled)
{
    BOOL_T  ret = FALSE;

    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if (SWCTRL_LogicalPortExisting(ifidx))
    {
        ret = AMTR_OM_SetMacNotifyPortStatus(ifidx, is_enabled);
    }

    return ret;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetRunningMacNotifyInterval
 *------------------------------------------------------------------------
 * FUNCTION: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *           non-default interval can be retrieved successfully. Otherwise,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT   : None
 * OUTPUT  : interval_p   - the non-default interval
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE    : 1. This function shall only be invoked by CLI to save the
 *            "running configuration" to local or remote files.
 *           2. Since only non-default configuration will be saved, this
 *            function shall return non-default interval.
 * -----------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T AMTR_MGR_GetRunningMacNotifyInterval(UI32_T  *interval_p)
{
    SYS_TYPE_Get_Running_Cfg_T  ret = SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if((amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE) || (NULL == interval_p))
    {
        AMTR_MGR_RETURN(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    if (TRUE == AMTR_OM_GetMacNotifyInterval(interval_p))
    {
        *interval_p /= SYS_BLD_TICKS_PER_SECOND;

        if (*interval_p == SYS_DFLT_AMTR_MAC_NOTIFY_TRAP_INTERVAL)
        {
            ret = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
        }
        else
        {
            ret = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
    }

    return ret;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetRunningMacNotifyGlobalStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *           non-default global status can be retrieved successfully. Otherwise,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT   : None
 * OUTPUT  : is_enabled_p   - the non-default global status
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE    : 1. This function shall only be invoked by CLI to save the
 *            "running configuration" to local or remote files.
 *           2. Since only non-default configuration will be saved, this
 *            function shall return non-default global status.
 * -----------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T AMTR_MGR_GetRunningMacNotifyGlobalStatus(BOOL_T  *is_enabled_p)
{
    SYS_TYPE_Get_Running_Cfg_T  ret = SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if((amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE) ||(NULL == is_enabled_p))
    {
        AMTR_MGR_RETURN(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    if(FALSE == AMTR_OM_GetMacNotifyGlobalStatus(is_enabled_p))
        return ret;

    if(SYS_DFLT_AMTR_MAC_NOTIFY_GLOBAL_STATUS == *is_enabled_p)
    {
        ret = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        ret = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    return ret;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_GetRunningMacNotifyPortStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *           non-default port status can be retrieved successfully. Otherwise,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT   : ifidx          - lport ifindex
 * OUTPUT  : is_enabled_p   - the non-default port status
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE    : 1. This function shall only be invoked by CLI to save the
 *            "running configuration" to local or remote files.
 *           2. Since only non-default configuration will be saved, this
 *            function shall return non-default port status.
 * -----------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T AMTR_MGR_GetRunningMacNotifyPortStatus(
    UI32_T  ifidx,
    BOOL_T  *is_enabled_p)
{
    SYS_TYPE_Get_Running_Cfg_T  ret = SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if((amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE) ||(NULL == is_enabled_p))
    {
        AMTR_MGR_RETURN(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    if(FALSE == AMTR_OM_GetMacNotifyPortStatus(ifidx, is_enabled_p))
    {
        return ret;
    }

    if (SYS_DFLT_AMTR_MAC_NOTIFY_PORT_STATUS == *is_enabled_p)
    {
        ret = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        ret = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    return ret;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_AddFstTrkMbr_CallBack
 * ------------------------------------------------------------------------
 * FUNCTION: Service the callback from SWCTRL when the port joins the trunk
 *           as the 1st member
 * INPUT   : trunk_ifindex  - specify which trunk to join.
 *           member_ifindex - specify which member port being add to trunk.
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : member_ifindex is sure to be a normal port asserted by SWCTRL.
 * ------------------------------------------------------------------------
 */
void AMTR_MGR_MacNotifyAddFstTrkMbr_CallBack(UI32_T  trunk_ifindex, UI32_T  member_ifindex)
{
    AMTR_OM_MacNotifyAddFstTrkMbr(trunk_ifindex, member_ifindex);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_DelTrkMbr_CallBack
 * ------------------------------------------------------------------------
 * FUNCTION: Service the callback from SWCTRL when the trunk member is
 *           removed from the trunk
 * INPUT   : trunk_ifindex  - specify which trunk to remove from
 *           member_ifindex - specify which member port being removed from
 *                            trunk
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
void AMTR_MGR_MacNotifyDelTrkMbr_CallBack(UI32_T  trunk_ifindex, UI32_T  member_ifindex)
{
    AMTR_OM_MacNotifyDelTrkMbr(trunk_ifindex, member_ifindex);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_DelLstTrkMbr_CallBack
 * ------------------------------------------------------------------------
 * FUNCTION: Service the callback from SWCTRL when the last trunk member
 *           is removed from the trunk
 * INPUT   : trunk_ifindex   -- specify which trunk to join to
 *          member_ifindex  -- specify which member port being add to trunk
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
void AMTR_MGR_MacNotifyDelLstTrkMbr_CallBack(UI32_T  trunk_ifindex, UI32_T  member_ifindex)
{
    AMTR_OM_MacNotifyDelLstTrkMbr(trunk_ifindex, member_ifindex);
}
#endif /* #if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE) */

#if (SYS_CPNT_AMTR_LOG_HASH_COLLISION_MAC == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_MGR_ClearCollisionVlanMacTable
 *------------------------------------------------------------------------------
 * PURPOSE  : Remove all entries in the collision mac table.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_ClearCollisionVlanMacTable(void)
{
    return AMTRDRV_MGR_ClearCollisionVlanMacTable();
}
#endif

#if (SYS_CPNT_AMTR_VLAN_MAC_LEARNING == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_MGR_GetVlanLearningStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get vlan learning status of specified vlan
 * INPUT    : vid
 * OUTPUT   : *learning_p
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_GetVlanLearningStatus(UI32_T vid, BOOL_T *learning_p)
{
    BOOL_T  ret = FALSE;

    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE || (NULL == learning_p))
        AMTR_MGR_RETURN(FALSE);

    ret = AMTR_OM_GetVlanLearningStatus(vid, learning_p);

    return ret;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_MGR_GetRunningVlanLearningStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get running vlan learning status of specified vlan
 * INPUT    : vid
 * OUTPUT   : *learning_p
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T AMTR_MGR_GetRunningVlanLearningStatus(UI32_T vid, BOOL_T *learning_p)
{
    SYS_TYPE_Get_Running_Cfg_T  ret = SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if((amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE) || (NULL == learning_p))
    {
        AMTR_MGR_RETURN(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    if(FALSE == AMTR_OM_GetVlanLearningStatus(vid, learning_p))
    {
        return ret;
    }

    if (SYS_DFLT_AMTR_VLAN_MAC_LEARNING == *learning_p)
    {
        ret = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        ret = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    return ret;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_MGR_SetVlanLearningStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Enable/disable vlan learning of specified vlan
 * INPUT    : vid
 *            learning
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_SetVlanLearningStatus(UI32_T vid, BOOL_T learning)
{
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if (TRUE == VLAN_OM_IsVlanExisted(vid))
    {
        if(FALSE == SWDRV_SetVlanLearningStatus(vid, learning))
            AMTR_MGR_RETURN(FALSE);

        if(FALSE == AMTR_MGR_DeleteAddrByVidAndLifeTime(vid, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT))
            AMTR_MGR_RETURN(FALSE);
    }

    LAN_SetVlanLearningStatus(vid, learning);

    AMTR_OM_SetVlanLearningStatus(vid, learning);

    return TRUE;
}
#endif
#if (SYS_CPNT_MLAG == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_SetMlagMacNotifyPortStatus
 *------------------------------------------------------------------------
 * FUNCTION: To set the MLAG mac notify port status
 * INPUT   : ifidx      - lport ifindex
 * OUTPUT  : is_enabled - port status to set
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -----------------------------------------------------------------------
 */
BOOL_T AMTR_MGR_SetMlagMacNotifyPortStatus(UI32_T  ifidx, BOOL_T  is_enabled)
{
    BOOL_T  ret = FALSE;

    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);

    if (SWCTRL_LogicalPortExisting(ifidx))
    {
        ret = AMTR_OM_SetMlagMacNotifyPortStatus(ifidx, is_enabled);
    }

    return ret;
}
/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_MlagMacNotifyProcessQueue
 *------------------------------------------------------------------------------
 * Purpose  : To process the MLAG mac notify entries in the queue
 * INPUT    : cur_time_stamp - current time stamp in ticks
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
void  AMTR_MGR_MlagMacNotifyProcessQueue(UI32_T  cur_time_stamp)
{
    AMTR_MGR_MacNotifyRec_T  *rec_p;
    UI32_T  time_passed, cur_pos, ntfy_cnt = 0, used_head = 0, idx, mac_ntfy_time_stamp = 0;
    UI32_T  mac_ntfy_interval = SYS_DFLT_AMTR_MAC_NOTIFY_TRAP_INTERVAL*SYS_BLD_TICKS_PER_SECOND;
    BOOL_T  is_enabled = FALSE, is_send = FALSE;

    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN();

    AMTR_OM_GetMlagMacNotifyTimeStamp(&mac_ntfy_time_stamp);
    if (mac_ntfy_time_stamp > cur_time_stamp)
    {
        time_passed = (0xFFFFFFFF - mac_ntfy_time_stamp) + cur_time_stamp;
    }
    else
    {
        time_passed = cur_time_stamp - mac_ntfy_time_stamp;
    }

    is_send = (time_passed >= mac_ntfy_interval) ? TRUE : FALSE;

    if (TRUE == is_send)
    {
        AMTR_OM_MlagMacNotifyCopyAndClearQueue(mac_ntfy_rec_lst, &ntfy_cnt, &used_head);

        for (idx = 0; idx < ntfy_cnt; idx ++)
        {
            cur_pos = (used_head+idx) % AMTR_MGR_MAC_NTFY_LST_MAX;
            rec_p = &mac_ntfy_rec_lst[cur_pos];

            /* MlagMacNotifyWA
             * This MlagMacNotify mechanism is limit by mac_ntfy_rec_lst size,
             * excess of changed entries can't be tracked.
             * Workaround is to send notification via MacAddrUpdateCallback.
             */
#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY_FOR_MLAG != TRUE)
            SYS_CALLBACK_MGR_MlagMacUpdateCallback(SYS_MODULE_AMTR, SYS_BLD_MLAG_GROUP_IPCMSGQ_KEY, rec_p->ifidx, rec_p->act_vid & AMTR_MGR_MAC_NTFY_VID_MASK, rec_p->src_mac, (rec_p->act_vid & AMTR_MGR_MAC_NTFY_ACT_MASK) ?  TRUE : FALSE);
#endif
        }

        if(amtr_mgr_bd_print_msg && ntfy_cnt != 0)
            BACKDOOR_MGR_Printf("%s:%d: head/mlag_ntfy_cnt-%ld/%ld\r\n", __FUNCTION__, __LINE__, used_head, ntfy_cnt);
    }

    return;
}
#endif

#if (SYS_CPNT_OVSVTEP == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_OvsMacNotifyProcessQueue
 *------------------------------------------------------------------------------
 * Purpose  : To process the MLAG mac notify entries in the queue
 * INPUT    : cur_time_stamp - current time stamp in ticks
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
void  AMTR_MGR_OvsMacNotifyProcessQueue(UI32_T cur_time_stamp)
{
    AMTR_MGR_MacNotifyRec_T  *rec_p;
    UI32_T  time_passed, cur_pos, ntfy_cnt = 0, used_head = 0, idx, mac_ntfy_time_stamp = 0;
    UI32_T  mac_ntfy_interval = SYS_DFLT_AMTR_MAC_NOTIFY_TRAP_INTERVAL*SYS_BLD_TICKS_PER_SECOND;
    BOOL_T  is_enabled = FALSE, is_send = FALSE;

    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN();

    AMTR_OM_OvsGetMacNotifyTimeStamp(&mac_ntfy_time_stamp);
    if (mac_ntfy_time_stamp > cur_time_stamp)
    {
        time_passed = (0xFFFFFFFF - mac_ntfy_time_stamp) + cur_time_stamp;
    }
    else
    {
        time_passed = cur_time_stamp - mac_ntfy_time_stamp;
    }

    is_send = (time_passed >= mac_ntfy_interval) ? TRUE : FALSE;

    if (TRUE == is_send)
    {
        AMTR_OM_OvsCopyAndClearQueue(mac_ntfy_rec_lst, &ntfy_cnt, &used_head);

        for (idx = 0; idx < ntfy_cnt; idx ++)
        {
            cur_pos = (used_head+idx) % AMTR_MGR_MAC_NTFY_LST_MAX;
            rec_p = &mac_ntfy_rec_lst[cur_pos];

            SYS_CALLBACK_MGR_OvsMacUpdateCallback(SYS_MODULE_AMTR, SYS_BLD_OVSVTEP_GROUP_IPCMSGQ_KEY, rec_p->ifidx, rec_p->act_vid & 0x7fff, rec_p->src_mac, (rec_p->act_vid & AMTR_MGR_MAC_NTFY_ACT_MASK) ?  TRUE : FALSE);
        }

        if(amtr_mgr_bd_print_msg && ntfy_cnt != 0)
            BACKDOOR_MGR_Printf("%s:%d: head/ntfy_cnt-%ld/%ld\r\n", __FUNCTION__, __LINE__, used_head, ntfy_cnt);
    }
}
#endif

/* LOCAL SUBPROGRAM BODIES
 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_UpdatePortSecurityStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will update port security status
 * INPUT   : ifindex    -- 0 means all port
 * OUTPUT  : none
 * RETURN  : none
 * -------------------------------------------------------------------------*/
static void AMTR_MGR_UpdatePortSecurityStatus(UI32_T ifindex)
{
    UI32_T start_ifindex, end_ifindex;

    if (ifindex != 0)
    {
        start_ifindex = end_ifindex = ifindex;
    }
    else
    {
        start_ifindex = 1;
        end_ifindex = SYS_ADPT_TOTAL_NBR_OF_LPORT;
    }

    for (ifindex = start_ifindex; ifindex <= end_ifindex; ifindex++)
    {
        if (IS_IFINDEX_INVALID(ifindex))
            continue;

        if (!AMTR_MGR_IsPortSecurityEnabled(ifindex)&&
            (amtr_port_info[ifindex-1].learn_with_count!=0))
        {
            SWCTRL_SetPortSecurityStatus(ifindex, VAL_portSecPortStatus_disabled, SWCTRL_PORT_SECURITY_ENABLED_BY_NONE);
        }
    }

    return;
}

#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_Notify_IntrusionMac
 * -------------------------------------------------------------------------
 * FUNCTION: When detecting intrusion mac, AMTR will notify other CSCs by this function.
 * INPUT   : vid    -- which vlan id
 *           mac    -- mac address
 *           ifindex-- which port
 *           is_age -- learned / aged
 * OUTPUT  : None
 * RETURN  : TRUE   -- Is intrusion
 *           FALSE  -- Is not intrusion
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static BOOL_T AMTR_MGR_Notify_IntrusionMac(UI32_T src_lport, UI16_T vid, UI8_T *src_mac, UI8_T *dst_mac, UI16_T ether_type)
{
    BOOL_T ret;

    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN(FALSE);


#ifdef AMTR_MGR_BACKDOOR_OPEN
    if(amtr_mgr_bd_print_msg)
        BACKDOOR_MGR_Printf("\r\n AMTR_MGR_Notify_IntrusionMac()");
#endif

    ret = NETACCESS_GROUP_IntrusionMacCallbackHandler(src_lport, vid, src_mac, dst_mac, ether_type);

/* remove here bcz SYS_CALLBACK can not get the true return code from remote API,
 *   and no other CSC need this callback, yet...
 */
#if 0
    ret = SYS_CALLBACK_MGR_IntrusionMacCallback(SYS_MODULE_AMTR,
               src_lport, vid, src_mac, dst_mac, ether_type);
#endif

    AMTR_MGR_RETURN(ret);
} /* End of AMTR_MGR_Notify_IntrusionMac() */

#endif/*end of #if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)*/

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_Notify_PortMove
 * -------------------------------------------------------------------------
 * FUNCTION: When port move, AMTR will notify other CSCs by this function.
 * INPUT   : num_of_entry  -- number of port move entries
 *           entry_p       -- port move entries buffer
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void AMTR_MGR_Notify_PortMove(UI32_T num_of_entry,
                                     AMTR_TYPE_PortMoveEntry_T *entry_p)
{
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN();

#ifdef AMTR_MGR_BACKDOOR_OPEN
    if(amtr_mgr_bd_print_msg)
        BACKDOOR_MGR_Printf("\r\n AMTR_MGR_Notify_PortMove()");
#endif

    SYS_CALLBACK_MGR_PortMoveCallback(SYS_MODULE_AMTR, num_of_entry, (UI8_T*)entry_p, num_of_entry*sizeof(AMTR_TYPE_PortMoveEntry_T));
    AMTR_MGR_RETURN();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_Notify_SecurityPortMove
 * -------------------------------------------------------------------------
 * FUNCTION: When port move, AMTR will notify other CSCs by this function.
 * INPUT   : ifindex          -- port whcih the mac is learnt now
 *           vid              -- which vlan id
 *           mac              -- mac address
 *           original_ifindex -- original port which the mac was learnt before
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void AMTR_MGR_Notify_SecurityPortMove(  UI32_T ifindex,
                                               UI32_T vid,
                                               UI8_T  *mac,
                                               UI32_T original_ifindex)
{
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN();


#ifdef AMTR_MGR_BACKDOOR_OPEN
    if(amtr_mgr_bd_print_msg)
        BACKDOOR_MGR_Printf("\r\n AMTR_MGR_Notify_SecurityPortMove()");
#endif

    /* if standalone of master unit
     */
    NETACCESS_GROUP_SecurityPortMoveCallbackHandler(ifindex, vid, mac, original_ifindex);
    SYS_CALLBACK_MGR_SecurityPortMoveCallback(SYS_MODULE_AMTR, ifindex, vid, mac, original_ifindex);
    AMTR_MGR_RETURN();
} /* End of AMTR_MGR_Notify_SecurityPortMove() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_Notify_AutoLearnCount
 * -------------------------------------------------------------------------
 * FUNCTION: When learning arrive learn_with_count, AMTR will notify other CSCs by this function.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void AMTR_MGR_Notify_AutoLearnCount(UI32_T ifindex, UI32_T portsec_status)
{
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
        AMTR_MGR_RETURN();

    NETACCESS_GROUP_AutoLearnCallbackHandler(ifindex, portsec_status);
    SYS_CALLBACK_MGR_AutoLearnCallback(SYS_MODULE_AMTR, ifindex, portsec_status);
    AMTR_MGR_RETURN();
} /* End of AMTR_MGR_Notify_AutoLearnCount() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_Notify_MACTableDeleteByPort
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when delete by port
 * INPUT   : vid                -- which vlan id
 *           mac                -- mac address
 *           new_port_ifindex   -- new port
 *           old_port_ifindex   -- old port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void AMTR_MGR_Notify_MACTableDeleteByPort(UI32_T ifindex, UI32_T reason)
{
    /* the event only be notified on IVL mode
     */
    if (amtr_mgr_vlan_learning_mode != VAL_dot1qConstraintType_independent)
        AMTR_MGR_RETURN();
    /* if standalone of master unit
     */
    NETACCESS_GROUP_MACTableDeleteByPortCallbackHandler(ifindex, reason);
    SYS_CALLBACK_MGR_MACTableDeleteByPortCallback(SYS_MODULE_AMTR, ifindex, reason);
    AMTR_MGR_RETURN();
} /* End of AMTR_MGR_Notify_EditAddrEntry() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_Notify_DeleteByVid
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when delete by vid
 * INPUT   : vid                -- which vlan id
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void AMTR_MGR_Notify_MACTableDeleteByVid(UI32_T vid)
{
    /* the event only be notified on IVL mode
     */
    if (amtr_mgr_vlan_learning_mode != VAL_dot1qConstraintType_independent)
        AMTR_MGR_RETURN();
    /* if standalone of master unit
     */
    NETACCESS_GROUP_MACTableDeleteByVidCallbackHandler(vid);
    SYS_CALLBACK_MGR_MACTableDeleteByVidCallback(SYS_MODULE_AMTR, vid);
    AMTR_MGR_RETURN();
} /* End of AMTR_MGR_Notify_EditAddrEntry() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_Notify_DeleteByVIDnPort
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when delete by vid+port
 * INPUT   : vid                -- which vlan id
 *           ifindex
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void AMTR_MGR_Notify_MACTableDeleteByVIDnPort(UI32_T vid, UI32_T ifindex)
{
    /* the event only be notified on IVL mode
     */
    if (amtr_mgr_vlan_learning_mode != VAL_dot1qConstraintType_independent)
        AMTR_MGR_RETURN();
    /* if standalone of master unit
     */
    NETACCESS_GROUP_MACTableDeleteByVIDnPortCallbackHandler(vid, ifindex);
    SYS_CALLBACK_MGR_MACTableDeleteByVIDnPortCallback(SYS_MODULE_AMTR, vid, ifindex);
    AMTR_MGR_RETURN();
} /* End of AMTR_MGR_Notify_EditAddrEntry() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_Notify_MacTableDeleteByLifeTime
 * -------------------------------------------------------------------------
 * FUNCTION: Call callback function, when delete by LifeTime
 * INPUT   : life_time  --- life time
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void AMTR_MGR_Notify_MacTableDeleteByLifeTime(UI32_T life_time)
{
    /* the event only be notified on IVL mode
     */
    if (amtr_mgr_vlan_learning_mode != VAL_dot1qConstraintType_independent)
        AMTR_MGR_RETURN();

    SYS_CALLBACK_MGR_MacTableDeleteByLifeTimeCallback(SYS_MODULE_AMTR, life_time);
}


#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_MGR_Notify_AgingOut
 *------------------------------------------------------------------------------
 * PURPOSE: Notify AMTR to delete this entry who needs to be aged out
 * INPUT  : UI32_T                num_of_entries  -- how many records in the buffer
 *          AMTR_TYPE_AddrEntry_T *addr_entry     -- The aged addresses
 * OUTPUT : None
 * RETURN : None
 * NOTES  :
 *------------------------------------------------------------------------------*/
static void AMTR_MGR_Notify_AgingOut(UI32_T num_of_entries, AMTR_TYPE_AddrEntry_T addr_buf[])
{
    if (FALSE == SYS_CALLBACK_MGR_AgeOutMacAddress(SYS_MODULE_AMTRDRV,num_of_entries,(UI8_T *)addr_buf,num_of_entries*sizeof(AMTR_TYPE_AddrEntry_T)))
    {
        BACKDOOR_MGR_Printf("%s: SYS_CALLBACK_MGR_AgeOutMacAddress return false\n",__FUNCTION__) ;
    }
    return;
}
#endif

/*----------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_Notify_SetStaticMACCheck
 *----------------------------------------------------------------------------
 * FUNCTION: Call call-back function, to check if setting(add/delete) a static MAC or not
 * INPUT   : vid,
 *           mac,
 *           ifindex,
 * OUTPUT  : None
 * RETURN  : FALSE, ignore the static MAC
 *           TRUE , set (add/delete) the static MAC
 * NOTE    : This function will be called iff static mac setting.
 *----------------------------------------------------------------------------*/
static BOOL_T AMTR_MGR_Notify_SetStaticMACCheck(UI32_T vid, UI8_T *mac, UI32_T ifindex)
{
    return NETACCESS_GROUP_SetStaticMacCheckCallbackHandler(vid, mac, ifindex);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_Notify_EditAddrEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when a address was learned/aged
 * INPUT   : vid    -- which vlan id
 *           mac    -- mac address
 *           ifindex-- which port
 *           is_age -- learned / aged
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void AMTR_MGR_Notify_EditAddrEntry(UI32_T vid, UI8_T *mac, UI32_T ifindex, BOOL_T is_age)
{
    NETACCESS_GROUP_EditAddrEntryCallbackHandler(vid, mac, ifindex, is_age);
    SYS_CALLBACK_MGR_MacAddrUpdateCallback(SYS_MODULE_AMTR, ifindex, (UI16_T)vid, mac, !is_age);
    return;
}

#if (AMTR_SUPPORT_NOTIFY_EDIT_ADDR_ENTRY_FOR_MLAG == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_Notify_EditAddrEntryForMlag
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when a address was learned/aged
 * INPUT   : vid    -- which vlan id
 *           mac    -- mac address
 *           ifindex-- which port
 *           is_age -- learned / aged
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void AMTR_MGR_Notify_EditAddrEntryForMlag(UI32_T vid, UI8_T *mac, UI32_T ifindex, BOOL_T is_age)
{
    BOOL_T is_mlag_port_enabled = FALSE;

    AMTR_OM_GetMlagMacNotifyPortStatus(ifindex, &is_mlag_port_enabled);

    if (is_mlag_port_enabled)
    {
        SYS_CALLBACK_MGR_MlagMacUpdateCallback(SYS_MODULE_AMTR, SYS_BLD_MLAG_GROUP_IPCMSGQ_KEY, ifindex, vid, mac, !is_age);
    }
}
#endif

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - AMTR_MGR_LPortToOctet
 * ------------------------------------------------------------------------|
 * FUNCTION : Logical Port to Octet String.
 * INPUT    : UI8_T *octet : Buffer to store Octet string
 *            UI32_T index : Logical port no.
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------*/
static void AMTR_MGR_LPortToOctet(UI8_T * octet, UI32_T index)
{
    UI8_T       tmp[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

    amtrmgr_memset(tmp, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    if (index == 0)
    {
        amtrmgr_memcpy(octet, tmp, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);;
        AMTR_MGR_RETURN();
    }
    amtrmgr_memset(tmp, 0x00, (index-1)/8);
    /* shift
     */
    amtrmgr_memset(tmp+(index-1)/8, 0x80 >> ((index+7) % 8), 1);
    amtrmgr_memset(tmp+(index-1)/8+1, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST - (index-1)/8 -1);
    amtrmgr_memcpy(octet, tmp, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
}/* End of AMTR_MGR_LPortToOctet() */


/*-------------------------------------------------------------------------|
 * ROUTINE NAME - AMTR_MGR_OctetToLPort
 * ------------------------------------------------------------------------|
 * FUNCTION : Octet String to Logical Port.
 * INPUT    : UI8_T *octet : Octet string
 *            UI32_T  comparing_length : string length
 * OUTPUT   : None
 * RETURN   : Which bit value is 1.(i.e.Logical port number)
 * NOTE     : None
 * ------------------------------------------------------------------------*/
static UI32_T AMTR_MGR_OctetToLPort(UI8_T * octet, UI32_T comparing_length)
{
    UI32_T      index;
    UI32_T      value;
    UI8_T       tmp[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

    amtrmgr_memset(tmp, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    if (amtrmgr_memcmp(tmp, octet, comparing_length) == 0)
        return 0;

    amtrmgr_memcpy(tmp, octet, comparing_length);
    for (index = 0; index < comparing_length; index++)
    {
      value = index*8+1;
      while (!(tmp[index] & 0x80) && (tmp[index] != 0x00) )
      {
         value++;
         tmp [index] = (UI8_T) (tmp[index] << 1);
      }

      if (tmp[index] & 0x80)
         return (value);
   }

   return 0;

}/* End of AMTR_MGR_OctetToLPort() */
/*-------------------------------------------------------------------------|
 * ROUTINE NAME -AMTR_MGR_NABufferDequeue
 * ------------------------------------------------------------------------|
 * FUNCTION : Restore NA to local buffer
 * INPUT    : UI32_T *num_of_entries : NA counters
 *            AMTR_TYPE_AddrEntry  addr_buff : NA buffer pointer
 * OUTPUT   : None
 * RETURN   : TRUE: Enqueue succeed else Failed
 * NOTE     : None
 * ------------------------------------------------------------------------*/

BOOL_T AMTR_MGR_NABufferDequeue(UI32_T * num_of_entries,AMTR_TYPE_AddrEntry_T addr_buf [ ]){
    return AMTR_OM_NABufferDequeue(num_of_entries,addr_buf);
}

#if (AMTR_MGR_STP_VALIDATION == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_CheckStpState
 *------------------------------------------------------------------------------
 * Purpose  : This function will check if stp port state is valid to
 *            learn MAC address.
 * INPUT    : vid
 *            lport
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
static BOOL_T AMTR_MGR_CheckStpState(UI32_T vid, UI32_T lport)
{
    UI32_T xstp_state;

#if (SYS_CPNT_VXLAN == TRUE)
    if(!AMTR_TYPE_LOGICAL_VID_IS_DOT1Q_VID(vid))
    {
        return TRUE;
    }
#endif

    if (!XSTP_OM_GetPortStateByVlan(vid, lport, &xstp_state))
    {
        return FALSE;
    }

    if (xstp_state != VAL_dot1dStpPortState_forwarding &&
        xstp_state != VAL_dot1dStpPortState_learning)
    {
        return FALSE;
    }

    return TRUE;
}
#endif /* (AMTR_MGR_STP_VALIDATION == TRUE) */

/*------------------------------------------------------------------------------
 * Function : AMTR_MGR_MacNotifyAddNewEntry
 *------------------------------------------------------------------------------
 * Purpose  : To add a new entry to the queue for further processing
 * INPUT    : ifidx   - lport ifindex
 *            vid     - vlan id
 *            src_mac - source mac
 *            is_add  - add/remove operation
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
static void AMTR_MGR_MacNotifyAddNewEntry(
    UI32_T  ifidx,
    UI16_T  vid,
    UI8_T   *src_mac_p,
    BOOL_T  is_add,
    UI8_T   skip_mac_notify)
{
#ifdef DEBUG_VXLAN
    printf("ifidx/vid/mac/add-%ld/%ld/%02x%02x%02x:%02x%02x%02x/%d\n",
        ifidx,vid,src_mac_p[0],src_mac_p[1],src_mac_p[2],
        src_mac_p[3],src_mac_p[4],src_mac_p[5],is_add);
#endif

#if (SYS_CPNT_VXLAN == TRUE)
    /* add vxlan mac of access port to kernel bridge fdb
     */
    if(AMTR_TYPE_LOGICAL_VID_IS_L_VFI_ID(vid))
    {
        L_INET_AddrIp_T     tmp_ip = {.type = L_INET_ADDR_TYPE_UNKNOWN};
        UI32_T              r_vxlan_port, access_lport, vid_ifidx;
        UI16_T              vxlan_vid, rc;

        VXLAN_TYPE_L_PORT_CONVERTTO_R_PORT(ifidx, r_vxlan_port);
        if (r_vxlan_port != 0)
        {
            if (TRUE == VXLAN_OM_GetVlanNlportOfAccessPort(r_vxlan_port, &vxlan_vid, &access_lport))
            {
                VLAN_OM_ConvertToIfindex(vxlan_vid, &vid_ifidx);

                /* vxlan mac dose not age out currently, so write static fdb to kernel
                 */
                rc = IPAL_NEIGH_AddNeighbor(
                    vid_ifidx, &tmp_ip, SYS_ADPT_MAC_ADDR_LEN, src_mac_p, IPAL_NEIGHBOR_TYPE_STATIC, TRUE);

#ifdef DEBUG_VXLAN
                printf("rc - %d\n", rc);
#endif
            }
        }
    }
#endif

#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
    BOOL_T  is_mac_notify_enabled = FALSE, is_mac_notify_port_enabled = FALSE;
#endif
#if (SYS_CPNT_MLAG == TRUE)
    BOOL_T  is_mlag_port_enabled = FALSE;
#endif

#if (SYS_CPNT_OVSVTEP == TRUE)
    if(!skip_mac_notify)
    {
        if(AMTR_TYPE_LOGICAL_VID_IS_L_VFI_ID(vid))
        {
            UI16_T r_vfi;

            if(AMTRDRV_OM_ConvertLvfiToRvfi(vid, &r_vfi))
            {
                AMTR_OM_OvsAddNewEntry(ifidx, r_vfi, src_mac_p, is_add, TRUE);
            }
        }
    }
#endif

#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
    if(!(skip_mac_notify & AMTR_MGR_MAC_NTFY_TRAP))
    {
#if(SYS_CPNT_VXLAN == TRUE)
        /*
         * Only support logical vid which belongs to dot1q vlan id
         */
        if(!AMTR_TYPE_LOGICAL_VID_IS_DOT1Q_VID(vid))
            return;
#endif
        AMTR_OM_GetMacNotifyGlobalStatus(&is_mac_notify_enabled);
        AMTR_OM_GetMacNotifyPortStatus(ifidx, &is_mac_notify_port_enabled);

        if(TRUE == is_mac_notify_enabled && TRUE == is_mac_notify_port_enabled)
        {
            AMTR_OM_MacNotifyAddNewEntry(ifidx, vid, src_mac_p, is_add, TRUE/*need sem*/);
        }
    }
#endif
#if (SYS_CPNT_MLAG == TRUE)
    if(!(skip_mac_notify & AMTR_MGR_MAC_NTFY_MLAG))
    {
        AMTR_OM_GetMlagMacNotifyPortStatus(ifidx, &is_mlag_port_enabled);
        if(TRUE == is_mlag_port_enabled)
        {
            AMTR_OM_MlagMacNotifyAddNewEntry(ifidx, vid, src_mac_p, is_add, TRUE/*need sem*/);
        }
    }
#endif

    AMTR_MGR_RETURN();
}

#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - AMTR_MGR_MacNotifySendTrap
 * ------------------------------------------------------------------------
 * FUNCTION: send mac-notification-trap
 * INPUT   : ifidx   - lport ifidx for notification
 *           vid     - vlan id     for notification
 *           src_mac - source mac  for notification
 *           is_add  - add/remove  for notification
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
static void AMTR_MGR_MacNotifySendTrap(
    UI32_T  ifidx,
    UI16_T  vid,
    UI8_T   *src_mac_p,
    BOOL_T  is_add)
{
    TRAP_EVENT_TrapData_T   trap_data;

    if(amtr_mgr_bd_print_msg)
        BACKDOOR_MGR_Printf("%s-%d ifidx:%ld, vid:%d, is_add:%s\r\n", __FUNCTION__, __LINE__, ifidx, vid, (is_add==TRUE)?"TRUE":"FALSE");
    trap_data.community_specified  = FALSE;  /* send trap to all community */
    trap_data.trap_type            = TRAP_EVENT_MAC_NOTIFY;

    trap_data.u.mac_notify.ifindex = ifidx;
    trap_data.u.mac_notify.vid     = vid;
    memcpy(trap_data.u.mac_notify.src_mac, src_mac_p, SYS_ADPT_MAC_ADDR_LEN);

    if (TRUE == is_add)
        trap_data.u.mac_notify.action  = VAL_macNotifyAction_add;
    else
        trap_data.u.mac_notify.action  = VAL_macNotifyAction_remove;

#if (SYS_CPNT_TRAPMGMT == TRUE)
    TRAP_MGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_TRAP_ONLY);
#else
    SNMP_PMGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_TRAP_ONLY);
#endif

}
#endif /* #if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE) */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTR_MGR_AuthenticatePacket
 * -------------------------------------------------------------------------
 * FUNCTION: This API will handle the packet by authenticated result
 * INPUT   : src_mac     -- source mac address
 *           vid         -- VLAN id
 *           lport       -- logical port ifindex
 *           auth_result -- authentication result
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *--------------------------------------------------------------------------*/
static BOOL_T AMTR_MGR_AuthenticatePacket(
    UI8_T *src_mac,
    UI32_T vid,
    UI32_T lport,
    SYS_CALLBACK_MGR_AuthenticatePacket_Result_T auth_result)
{
    AMTR_TYPE_AddrEntry_T addr_entry;
    BOOL_T to_learn = TRUE;
    BOOL_T to_learn_permanent = FALSE;
    BOOL_T to_drop = FALSE;
    BOOL_T is_security_mac = FALSE;

    /* get current learning status
     */
    {
        UI32_T learning_disabled_status, intruder_handlers;

        if (!SWCTRL_GetPortLearningStatusEx(lport, &learning_disabled_status, &intruder_handlers))
        {
            return FALSE;
        }

        to_learn = !learning_disabled_status;
    }

    /* default behavior:
     *   learn with life_time == DELETE_ON_TIMEOUT and
     *   announce to upper layer
     */
    switch (auth_result)
    {
        /* result authenticated has not determined,
         * don't learn this mac now.
         */
        case SYS_CALLBACK_MGR_AUTH_PENDING:
            to_learn = FALSE;
            break;

        /* for authenticated packet,
         * learn with life_time == DELETE_ON_RESET
         */
        case SYS_CALLBACK_MGR_AUTH_AUTHENTICATED:
            to_learn_permanent = TRUE;
            is_security_mac = TRUE;
            break;

        /* for unauthenticated packet,
         * don't learn and discard this packet.
         */
        case SYS_CALLBACK_MGR_AUTH_UNAUTHENTICATED:
        case SYS_CALLBACK_MGR_AUTH_FAILED:
            to_learn = FALSE;
            to_drop = TRUE;
            break;

        /* use default behavior
         */
        case SYS_CALLBACK_MGR_AUTH_AUTHENTICATED_TEMPORARILY:
            is_security_mac=TRUE;
        case SYS_CALLBACK_MGR_AUTH_BYPASS:
        default:
            ;
    }

#if (SYS_CPNT_VRRP == TRUE)
    {
#define IS_VRRP_GROUP_MAC(mac) (((mac)[0]==0x00)&&((mac)[1]==0x00)&&((mac)[2]==0x5e)&& \
                               ((mac)[3]==0x00)&&((mac)[4]==0x01))

        if(IS_VRRP_GROUP_MAC(src_mac))
        {
            AMTR_TYPE_AddrEntry_T vmac_entry;

            /* When VRRP master receive advertisement,
             * the received vrrp packet will cause port move,
             * because VRRP master already add the same virtaul mac address as static mac.
             * The new learn virtual mac can't replace the self static mac,
             * we should bypass learning process and annouce this packet to VRRP.
             */
            memset(&vmac_entry, 0, sizeof(vmac_entry));
            vmac_entry.vid = vid;
            memcpy(vmac_entry.mac, src_mac, sizeof(vmac_entry.mac));
            if((TRUE == AMTR_MGR_GetExactAddrEntry(&vmac_entry))&&
               (vmac_entry.source == AMTR_TYPE_ADDRESS_SOURCE_SELF))
            {
                to_learn = FALSE;
            }
        }
    }
#endif

    if (to_learn)
    {
        memset(&addr_entry, 0, sizeof(addr_entry));
        memcpy(addr_entry.mac, src_mac, sizeof(addr_entry.mac));
        addr_entry.vid = vid;
        addr_entry.ifindex = lport;
        addr_entry.action = AMTR_TYPE_ADDRESS_ACTION_FORWARDING;
        addr_entry.source = (is_security_mac==TRUE)?AMTR_TYPE_ADDRESS_SOURCE_SECURITY:AMTR_TYPE_ADDRESS_SOURCE_LEARN;
        addr_entry.life_time = (to_learn_permanent ?
            AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET :
            AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);

        if (AMTR_MGR_SetAddrEntry(&addr_entry)==FALSE)
        {
            /* EPR:ECS4210-52P-00264
             * HeadLine:PortSecurity: After DUT learned 1024 security MAC,
             *          DUT will transmit multicast of the 1025th SA MAC.
             * Description: When the mac address cannot be set, the mac
             *              should be considered as authentication failed
             *              and drop it.
             */
            to_drop=TRUE;
        }
    }

    if (to_drop)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


#ifdef AMTR_MGR_BACKDOOR_OPEN

static void AMTR_MGR_BD_DumpHisam(void)
{
    UI8_T   mv_key[AMTR_MVKEY_LEN];
    amtrmgr_memset (mv_key, 0, AMTR_MVKEY_LEN );
    AMTR_OM_HisamSearch(AMTR_MV_KIDX, mv_key, AMTR_MGR_BD_DisplayCallback,0,(void *)0);
    return;
}

static UI32_T AMTR_MGR_BD_DisplayCallback (void *record, void *cookies)
{
    AMTR_TYPE_AddrEntry_T *hisam_entry;
    if (record == 0)
    {
        return L_HISAM_SEARCH_CONTINUE;
    }
    hisam_entry = (AMTR_TYPE_AddrEntry_T *) record;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("MAC:%02x:%02x:%02x:%02x:%02x:%02x ",
                            hisam_entry->mac[0],
                            hisam_entry->mac[1],
                            hisam_entry->mac[2],
                            hisam_entry->mac[3],
                            hisam_entry->mac[4],
                            hisam_entry->mac[5]);
    BACKDOOR_MGR_Printf("VID:%d ",      hisam_entry->vid);
    BACKDOOR_MGR_Printf("Ifindex:%d ",  hisam_entry->ifindex);
    BACKDOOR_MGR_Printf("Action:%d ",   hisam_entry->action);
    BACKDOOR_MGR_Printf("Source:%d ",  hisam_entry->source);
    BACKDOOR_MGR_Printf("LifeTime:%d", hisam_entry->life_time);
    BACKDOOR_MGR_Printf("Priority:%d ", hisam_entry->priority);
    return L_HISAM_SEARCH_CONTINUE;
}

static void AMTR_MGR_BD_DumpPortInfo(void)
{
    UI8_T   ch;
    UI32_T ifindex=0;
    AMTR_MGR_PortInfo_T port_info;

    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       Dump Port Info Table          ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n Input the ifindex:");

    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        BACKDOOR_MGR_Printf("%0x",ch);
        ch -= 0x30;
        if(ch>9)
        {
            BACKDOOR_MGR_Printf("Error Input %0x\r\n",ch);
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        ifindex = ifindex*10+ch;
    }
    AMTR_OM_GetPortInfo(ifindex, & port_info);
    BACKDOOR_MGR_Printf("\r\n--------------------AMTR Port Info Table--------------------------");
    BACKDOOR_MGR_Printf("\r\nifindex = %ld      life_time = %d       protocol = %d     learn_with_count:%ld"
             ,ifindex,port_info.life_time,port_info.protocol, port_info.learn_with_count);
    BACKDOOR_MGR_Printf("\r\n");
    return;
}

static void AMTR_MGR_BD_SetPortInfo(void)
{
    UI8_T   ch;
    UI32_T ifindex=0;
    AMTR_MGR_PortInfo_T port_info;

    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf(" \r\n      Set Port Info Table         ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n Which ifindex you want to set:");
    /* ifindex
     */
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch>9)
        {
            BACKDOOR_MGR_Printf("Error Input\r\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        ifindex = ifindex*10+ch;
    }

    /* Life Time
     */
    BACKDOOR_MGR_Printf("\r\n Set Life Time(1:other 2:invalid 3:permanent 4:del_reset 5:del_timeout):");
    port_info.life_time= 0;
    ch = BACKDOOR_MGR_GetChar();
    ch -= 0x30;
    BACKDOOR_MGR_Printf("%d", ch);
    port_info.life_time=ch;
    BACKDOOR_MGR_Printf("\r\n");

    /* protocol
     */
    BACKDOOR_MGR_Printf("\r\n Set protocol(0:normal 1:Psec 2:Dot1X):");
    port_info.protocol= 0;
    ch = BACKDOOR_MGR_GetChar();
    ch -= 0x30;
    BACKDOOR_MGR_Printf("%d", ch);
    port_info.protocol=ch;
    BACKDOOR_MGR_Printf("\r\n");

    /* learn_with_count
     */
    BACKDOOR_MGR_Printf("\r\n Set learn_with_count:");
    port_info.learn_with_count= 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch>9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\r\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        port_info.learn_with_count = port_info.learn_with_count*10+ch;
    }

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    if(AMTR_MGR_SetPortInfo(ifindex, &port_info))
        BACKDOOR_MGR_Printf("\r\nSet Port Info Table success!!");
    else
        BACKDOOR_MGR_Printf("\r\nSet Port Info Table Fail!!");

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);

    return;
}

static void AMTR_MGR_BD_SetAddrEntry(void)
{
    UI8_T   ch;
    UI8_T   index;
    UI8_T   mac[12];
    AMTR_TYPE_AddrEntry_T addr_entry;

    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       Set addrress entry         ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    /* vlan
     */
    BACKDOOR_MGR_Printf("\r\n Set VID:");
    addr_entry.vid = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch-=0x30;
        if(ch > 9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        addr_entry.vid = addr_entry.vid*10+ch;
    }

    /* ifindex
     */
    BACKDOOR_MGR_Printf("\r\n Set ifindex:");
    addr_entry.ifindex = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch>9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\r\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        addr_entry.ifindex = addr_entry.ifindex*10+ch;
    }

    /* mac
     */
    BACKDOOR_MGR_Printf("\r\n MAC addr:");
    index = 0;
    while((index < 12) && ((ch=BACKDOOR_MGR_GetChar()) != 0xd))
    {
        if(ch >= '0' && ch <= '9')
        {
            ch -= '0';
        }
        else if(ch >= 'A' && ch <='F')
        {
            ch = ch - 'A' + 0x0a;
        }
        else if(ch >= 'a' && ch <='f')
        {
            ch = ch - 'a' + 0x0a;
        }
        else
        {
            BACKDOOR_MGR_Printf("Error in Input\r\n");
            return;
        }
        BACKDOOR_MGR_Printf("%x", ch);
        mac[index++] = ch;
    }

    for(index = 0; index < 6; index++)
    {
        addr_entry.mac[index] = (((mac[index*2] & 0x0F)<<4 ) | (mac[index*2 + 1] & 0x0F));
    }

    BACKDOOR_MGR_Printf("\r\nThe MAC you enter is %02x-%02x-%02x-%02x-%02x-%02x\r\n",
        addr_entry.mac[0], addr_entry.mac[1], addr_entry.mac[2], addr_entry.mac[3], addr_entry.mac[4], addr_entry.mac[5]);

    /* action
     */
    BACKDOOR_MGR_Printf("\r\n Set Action(1:forward 2:discard_S 3:discard_D 4:CPU 5:For_and_CPU):");
    addr_entry.action = 0;
    ch = BACKDOOR_MGR_GetChar();
    ch -= 0x30;
    BACKDOOR_MGR_Printf("%d", ch);
    addr_entry.action=ch;
    BACKDOOR_MGR_Printf("\r\n");

    /* source
     */
    BACKDOOR_MGR_Printf("\r\n Set Source(1:other 2:invalid 3:learnt 4:self 5:config 6:security):");
    addr_entry.source= 0;
    ch = BACKDOOR_MGR_GetChar();
    ch -= 0x30;
    BACKDOOR_MGR_Printf("%d", ch);
    addr_entry.source=ch;
    BACKDOOR_MGR_Printf("\r\n");

    /* Life Time
     */
    BACKDOOR_MGR_Printf("\r\n Set Life Time(1:other 2:invalid 3:permanent 4:del_reset 5:del_timeout ):");
    addr_entry.life_time= 0;
    ch = BACKDOOR_MGR_GetChar();
    ch -= 0x30;
    BACKDOOR_MGR_Printf("%d", ch);
    addr_entry.life_time=ch;
    BACKDOOR_MGR_Printf("\r\n");

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    if(AMTR_MGR_SetAddrEntry(&addr_entry))
        BACKDOOR_MGR_Printf("Set Address Entry success!!\r\n");
    else
        BACKDOOR_MGR_Printf("Set Address Entry fail!!\r\n");

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);

    return;
}

static void AMTR_MGR_BD_SetAddrEntryWithPriority(void)
{
    UI8_T   ch;
    UI8_T   index;
    UI8_T   mac[12];
    AMTR_TYPE_AddrEntry_T addr_entry;

    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       Set addrress entry with priority         ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    /* vlan
     */
    BACKDOOR_MGR_Printf("\r\n Set VID:");
    addr_entry.vid = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch-=0x30;
        if(ch > 9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        addr_entry.vid = addr_entry.vid*10+ch;
    }

    /* ifindex
     */
    BACKDOOR_MGR_Printf("\r\n Set ifindex:");
    addr_entry.ifindex = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch>9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\r\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        addr_entry.ifindex = addr_entry.ifindex*10+ch;
    }

    /* mac
     */
    BACKDOOR_MGR_Printf("\r\n MAC addr:");
    index = 0;
    while((index < 12) && ((ch=BACKDOOR_MGR_GetChar()) != 0xd))
    {
        if(ch >= '0' && ch <= '9')
        {
            ch -= '0';
        }
        else if(ch >= 'A' && ch <='F')
        {
            ch = ch - 'A' + 0x0a;
        }
        else if(ch >= 'a' && ch <='f')
        {
            ch = ch - 'a' + 0x0a;
        }
        else
        {
            BACKDOOR_MGR_Printf("Error in Input\r\n");
            return;
        }
        BACKDOOR_MGR_Printf("%x", ch);
        mac[index++] = ch;
    }

    for(index = 0; index < 6; index++)
    {
        addr_entry.mac[index] = (((mac[index*2] & 0x0F)<<4 ) | (mac[index*2 + 1] & 0x0F));
    }

    BACKDOOR_MGR_Printf("\r\nThe MAC you enter is %02x-%02x-%02x-%02x-%02x-%02x\r\n",
        addr_entry.mac[0], addr_entry.mac[1], addr_entry.mac[2], addr_entry.mac[3], addr_entry.mac[4], addr_entry.mac[5]);

    /* action
     */
    BACKDOOR_MGR_Printf("\r\n Set Action(0:forward 1:discard_S 2:discard_D 3:CPU 4:For_and_CPU):");
    addr_entry.action = 0;
    ch = BACKDOOR_MGR_GetChar();
    ch -= 0x30;
    BACKDOOR_MGR_Printf("%d", ch);
    addr_entry.action=ch;
    BACKDOOR_MGR_Printf("\r\n");

    /* source
     */
    BACKDOOR_MGR_Printf("\r\n Set Source(1:other 2:invalid 3:learnt 4:self 5:config 6:security):");
    addr_entry.source= 0;
    ch = BACKDOOR_MGR_GetChar();
    ch -= 0x30;
    BACKDOOR_MGR_Printf("%d", ch);
    addr_entry.source=ch;
    BACKDOOR_MGR_Printf("\r\n");

    /* Life Time
     */
    BACKDOOR_MGR_Printf("\r\n Set Life Time(1:other 2:invalid 3:permanent 4:del_reset 5:del_timeout ):");
    addr_entry.life_time= 0;
    ch = BACKDOOR_MGR_GetChar();
    ch -= 0x30;
    BACKDOOR_MGR_Printf("%d", ch);
    addr_entry.life_time=ch;
    BACKDOOR_MGR_Printf("\r\n");

    /* priority
     */
    BACKDOOR_MGR_Printf("\r\n Set Priority:");
    addr_entry.priority = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch>9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\r\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        addr_entry.priority = addr_entry.priority*10+ch;
    }
    BACKDOOR_MGR_Printf("\r\n");

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    if(AMTR_MGR_SetAddrEntryWithPriority(&addr_entry))
        BACKDOOR_MGR_Printf("\nSet Address Entry success!!\r\n");
    else
        BACKDOOR_MGR_Printf("\nSet Address Entry fail!!\r\n");

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);

    return;
}

static void AMTR_MGR_BD_DelAddrEntry(void)
{
    UI8_T   mac[12];
    UI32_T vid;
    UI8_T ch;
    UI8_T   index;

    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       Delete address entry         ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    /* vlan
     */
    BACKDOOR_MGR_Printf("\r\n Set VID:");
    vid = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch > 9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        vid = vid*10+ch;
    }

    /* mac
     */
    BACKDOOR_MGR_Printf("\r\n MAC addr:");
    index = 0;
    while((index < 12) && ((ch=BACKDOOR_MGR_GetChar()) != 0xd))
    {
        if(ch >= '0' && ch <= '9')
        {
            ch -= '0';
        }
        else if(ch >= 'A' && ch <='F')
        {
            ch = ch - 'A' + 0x0a;
        }
        else if(ch >= 'a' && ch <='f')
        {
            ch = ch - 'a' + 0x0a;
        }
        else
        {
            BACKDOOR_MGR_Printf("Error in Input\r\n");
            return;
        }
        BACKDOOR_MGR_Printf("%x", ch);
        mac[index++] = ch;
    }

    for(index = 0; index < 6; index++)
    {
        mac[index] = (((mac[index*2] & 0x0F)<<4 ) | (mac[index*2 + 1] & 0x0F));
    }

    BACKDOOR_MGR_Printf("\r\nThe MAC you enter is %02x-%02x-%02x-%02x-%02x-%02x\r\n",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    if(AMTR_MGR_DeleteAddr(vid,mac))
        BACKDOOR_MGR_Printf("Delete Address Entry success!!\r\n");
    else
        BACKDOOR_MGR_Printf("Delete Address Entry Fail!!\r\n");

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);

    return;
}

static void AMTR_MGR_BD_DelAddrByPort(void)
{
    UI32_T ifindex;
    UI8_T ch;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       Delete All Address by port         ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n Set ifindex:");
    ifindex = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch>9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\r\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        ifindex = ifindex*10+ch;
    }

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    if(AMTR_MGR_DeleteAddrByLPort(ifindex))
        BACKDOOR_MGR_Printf("\r\ndelete by port success!!");
    else
        BACKDOOR_MGR_Printf("\r\ndelete by port Fail!!");

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);

    return;
}

static void AMTR_MGR_BD_DelAddrByVid(void)
{
    UI32_T vid;
    UI8_T ch;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       Delete All Address by Vid         ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    /* vlan
     */
    BACKDOOR_MGR_Printf("\r\n Set VID:");
    vid = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch > 9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        vid = vid*10+ch;
    }

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    if(AMTR_MGR_DeleteAddrByVID(vid))
        BACKDOOR_MGR_Printf("\r\ndelete address entry by vid success!!");
    else
        BACKDOOR_MGR_Printf("\r\ndelete address entry by vid Fail!!");

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);

    return;
}

static void AMTR_MGR_BD_DelAddrByLifeTime(void)
{
    UI32_T life_time;
    UI8_T   ch;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       Delete All Address Entry By Life Time      ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n");

    /* life time
     */
    BACKDOOR_MGR_Printf("\r\nLife Time(1.Other 2.Invalid 3.Permanent 4.Reset 5.Timeout):");
    ch=BACKDOOR_MGR_GetChar();
    ch -= 0x30;
    life_time=ch;

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    if(AMTR_MGR_DeleteAddrByLifeTime(life_time))
        BACKDOOR_MGR_Printf("\r\ndelete by life_time success!!");
    else
        BACKDOOR_MGR_Printf("\r\ndelete by life_time Fail!!");

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);

    return;
}

static void AMTR_MGR_BD_DelAddrBySource(void)
{
    UI32_T source;
    UI8_T   ch;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       Delete All Address Entry By Source     ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n");

    /* source
     */
    BACKDOOR_MGR_Printf("\r\n Set Source(1:other 2:invalid 3:learnt 4:self 5:config 6:security):");
    ch=BACKDOOR_MGR_GetChar();
    ch -= 0x30;
    source=ch;

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    if(AMTR_MGR_DeleteAddrBySource(source))
        BACKDOOR_MGR_Printf("\r\ndelete by source success!!");
    else
        BACKDOOR_MGR_Printf("\r\ndelete by source Fail!!");

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);

    return;
}

static void AMTR_MGR_BD_DelAddrByPortAndLifeTime(void)
{
    UI32_T ifindex;
    UI32_T life_time;
    UI8_T ch;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n        Delete Address By LifeTime And Port         ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n Set ifindex:");
    ifindex = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch>9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\r\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        ifindex = ifindex*10+ch;
    }

    /* life time
     */
    BACKDOOR_MGR_Printf("\r\nLife Time(1.Other 2.Invalid 3.Permanent 4.Reset 5.Timeout):");
    ch=BACKDOOR_MGR_GetChar();
    ch -= 0x30;
    life_time=ch;

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    if(AMTR_MGR_DeleteAddrByLifeTimeAndLPort(ifindex, life_time))
        BACKDOOR_MGR_Printf("\r\ndelete Dynamic address entry by port success!!");
    else
        BACKDOOR_MGR_Printf("\r\ndelete Dynamic address entry by port Fail!!");

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);

    return;
}

static void AMTR_MGR_BD_DelAddrByPortAndSource(void)
{
    UI32_T ifindex;
    UI32_T source;
    UI8_T ch;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n        Delete Address By Source And Port         ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n Set ifindex:");
    ifindex = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch>9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\r\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        ifindex = ifindex*10+ch;
    }

    /* source
     */
    BACKDOOR_MGR_Printf("\r\n Set Source(1:other 2:invalid 3:learnt 4:self 5:config 6:security):");
    ch=BACKDOOR_MGR_GetChar();
    ch -= 0x30;
    source=ch;

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    if(AMTR_MGR_DeleteAddrBySourceAndLPort(ifindex, source))
        BACKDOOR_MGR_Printf("\r\ndelete by port+source success!!");
    else
        BACKDOOR_MGR_Printf("\r\ndelete by port+source Fail!!");

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);

    return;
}

static void AMTR_MGR_BD_DelAddrByVidAndLifeTime(void)
{
    UI32_T vid;
    UI32_T life_time;
    UI8_T ch;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n        Delete Address By LifeTime And Vid        ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    /* vlan
     */
    BACKDOOR_MGR_Printf("\r\n Set VID:");
    vid = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch > 9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        vid = vid*10+ch;
    }

    /* life time
     */
    BACKDOOR_MGR_Printf("\r\nLife Time(1.Other 2.Invalid 3.Permanent 4.Reset 5.Timeout):");
    ch=BACKDOOR_MGR_GetChar();
    ch -= 0x30;
    life_time=ch;

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    if(AMTR_MGR_DeleteAddrByVidAndLifeTime(vid,life_time))
        BACKDOOR_MGR_Printf("\r\ndelete by vid+life_time success!!");
    else
        BACKDOOR_MGR_Printf("\r\ndelete by vid+life_time Fail!!");

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);

    return;
}

static void AMTR_MGR_BD_DelAddrByVidAndSource(void)
{
    UI32_T vid;
    UI32_T source;
    UI8_T ch;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n        Delete Address By Source And Port         ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n Set VID:");
    /* vlan
     */
    vid = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch > 9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        vid = vid*10+ch;
    }

    /* source
     */
    BACKDOOR_MGR_Printf("\r\n Set Source(1:other 2:invalid 3:learnt 4:self 5:config 6:security):");
    ch=BACKDOOR_MGR_GetChar();
    ch -= 0x30;
    source=ch;

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    if(AMTR_MGR_DeleteAddrByVidAndSource(vid, source))
        BACKDOOR_MGR_Printf("\r\ndelete by vid+source success!!");
    else
        BACKDOOR_MGR_Printf("\r\ndelete by vid+source Fail!!");

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);

    return;
}

static void AMTR_MGR_BD_DelAddrByLifeTimenVidnPort(void)
{
    UI32_T vid;
    UI32_T ifindex;
    UI32_T life_time;
    UI8_T ch;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       Delete Address By LifeTime And Vid And Port         ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    /* vlan
     */
    BACKDOOR_MGR_Printf("\r\n Set VID:");
    vid = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch > 9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        vid = vid*10+ch;
    }

    /* ifindex
     */
    BACKDOOR_MGR_Printf("\r\n Set ifindex:");
    ifindex = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch>9)
        {
            BACKDOOR_MGR_Printf("\r\nError Input");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        ifindex = ifindex*10+ch;
    }
    /* life time
     */
    BACKDOOR_MGR_Printf("\r\nLife Time(1.Other 2.Invalid 3.Permanent 4.Reset 5.Timeout):");
    ch=BACKDOOR_MGR_GetChar();
    ch -= 0x30;
    life_time=ch;

    if(AMTR_MGR_DeleteAddrByLifeTimeAndVidRangeAndLPort(ifindex, vid, vid, life_time))
        BACKDOOR_MGR_Printf("\r\nDelete by port+vid+life_time success!!");
    else
        BACKDOOR_MGR_Printf("\r\nDelete by port+vid+life_time Fail!!");

    return;
}


static void AMTR_MGR_BD_DelAddrBySourcenVidnPort(void)
{
    UI32_T vid;
    UI32_T ifindex;
    UI32_T source;
    UI8_T ch;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n        Delete Address By Source And Vid And Port        ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    /* vlan
     */
    BACKDOOR_MGR_Printf("\r\n Set VID:");
    vid = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch > 9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        vid = vid*10+ch;
    }

    /* ifindex
     */
    BACKDOOR_MGR_Printf("\r\n Set ifindex:");
    ifindex = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch>9)
        {
            BACKDOOR_MGR_Printf("\r\nError Input");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        ifindex = ifindex*10+ch;
    }

    /* source
     */
    BACKDOOR_MGR_Printf("\r\n Set Source(1:other 2:invalid 3:learnt 4:self 5:config 6:security):");
    ch=BACKDOOR_MGR_GetChar();
    ch -= 0x30;
    source=ch;

    if(AMTR_MGR_DeleteAddrBySourceAndVidAndLPort(ifindex, vid, source))
        BACKDOOR_MGR_Printf("\r\nDelete by port+vid+source success!!");
    else
        BACKDOOR_MGR_Printf("\r\nDelete by port+vid+source Fail!!");

    return;
}

static void AMTR_MGR_BD_DelAddrExceptByVidnPortCertainAddr(void)
{
    UI32_T vid;
    UI32_T ifindex;
    UI8_T mac_list_num;
    UI8_T mac_list_ar[3][SYS_ADPT_MAC_ADDR_LEN];
    UI8_T mask_list_ar[3][SYS_ADPT_MAC_ADDR_LEN];
    UI32_T i, v;
    UI8_T ch;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n        Delete Address By Vid And Port Except Certain Addr        ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    /* vlan
     */
    BACKDOOR_MGR_Printf("\r\n Set VID:");
    vid = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch > 9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        vid = vid*10+ch;
    }

    /* ifindex
     */
    BACKDOOR_MGR_Printf("\r\n Set ifindex:");
    ifindex = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch>9)
        {
            BACKDOOR_MGR_Printf("\r\nError Input");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        ifindex = ifindex*10+ch;
    }

    /* mac list
     */
    memset(mac_list_ar, 0, sizeof(mac_list_ar));
    memset(mask_list_ar, 0, sizeof(mask_list_ar));
    for (mac_list_num = 0; mac_list_num < sizeof(mac_list_ar)/SYS_ADPT_MAC_ADDR_LEN; mac_list_num++)
    {
        BOOL_T get_mask = FALSE;
        UI8_T *mac_p;
    get_mac_addr:
        if (get_mask)
        {
            BACKDOOR_MGR_Printf("\r\n Excluded MAC mask:");
            mac_p = mask_list_ar[mac_list_num];
        }
        else
        {
            BACKDOOR_MGR_Printf("\r\n Excluded MAC addr:");
            mac_p = mac_list_ar[mac_list_num];
        }
        for (i = 0, v = 0xff; i < 12; )
        {
            ch=BACKDOOR_MGR_GetChar();
            if (ch == 0xd && i == 0)
                goto get_mac_addr_finish;
            else if (ch == 0xd && i > 0)
                break;
            else if (isdigit(ch))
                v = ch - 0x30;
            else if (ch >= 'A' && ch <= 'F')
                v = ch - 'A' + 10;
            else if (ch >= 'a' && ch <= 'f')
                v = ch - 'a' + 10;
            if (v < 0x10)
            {
                BACKDOOR_MGR_Printf("%lX", v);
                if ((i & 1) == 1 && i != 11)
                    BACKDOOR_MGR_Printf("-");
                mac_p[i/2] |= v << ((i & 1)?0:4);
                i++;
                v = 0x10;
            }
        }
        if (i < 12)
        {
            BACKDOOR_MGR_Printf(" Error Input: ");
            goto get_mac_addr;
        }
        else if (!get_mask)
        {
            get_mask = TRUE;
            goto get_mac_addr;
        }
    }
get_mac_addr_finish:
    BACKDOOR_MGR_Printf("\r\n Number of MAC: %u", mac_list_num);

    if(AMTR_MGR_DeleteAddrByVidAndLPortExceptCertainAddr(ifindex, vid, mac_list_ar, mask_list_ar, mac_list_num))
        BACKDOOR_MGR_Printf("\r\nDelete by port+vid-addrs success!!");
    else
        BACKDOOR_MGR_Printf("\r\nDelete by port+vid-addrs Fail!!");

    return;
}

static void AMTR_MGR_BD_DelAllAddr(void)
{

    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       Delete All Address Entry         ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n");

    if(AMTR_MGR_DeleteAllAddr())
        BACKDOOR_MGR_Printf("delete all address entry success!!\r\n");
    else
        BACKDOOR_MGR_Printf("delete all address entry Fail!!\r\n");
    return;
}

static void AMTR_MGR_BD_GetAddr(void)
{
 UI8_T   mac[12];
    UI32_T vid;
    UI8_T ch;
    UI8_T   index;
    AMTR_TYPE_AddrEntry_T addr_entry;

    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       Get Address Entry         ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    /* vlan
     */
    BACKDOOR_MGR_Printf("\r\n Set VID:");
    vid = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch > 9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        vid = vid*10+ch;
    }

    /* mac
     */
    BACKDOOR_MGR_Printf("\r\n MAC addr:");
    index = 0;
    while((index < 12) && ((ch=BACKDOOR_MGR_GetChar()) != 0xd))
    {
        if(ch >= '0' && ch <= '9')
        {
            ch -= '0';
        }
        else if(ch >= 'A' && ch <='F')
        {
            ch = ch - 'A' + 0x0a;
        }
        else if(ch >= 'a' && ch <='f')
        {
            ch = ch - 'a' + 0x0a;
        }
        else
        {
            BACKDOOR_MGR_Printf("Error in Input\r\n");
            return;
        }
        BACKDOOR_MGR_Printf("%x", ch);
        mac[index++] = ch;
    }

    for(index = 0; index < 6; index++)
    {
        mac[index] = (((mac[index*2] & 0x0F)<<4 ) | (mac[index*2 + 1] & 0x0F));
    }

    BACKDOOR_MGR_Printf("\r\nThe MAC you enter is %02x-%02x-%02x-%02x-%02x-%02x\r\n",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    addr_entry.vid=vid;
    amtrmgr_memcpy(addr_entry.mac, mac, AMTR_TYPE_MAC_LEN);
    if(AMTR_MGR_GetExactAddrEntry(&addr_entry))
    {
        BACKDOOR_MGR_Printf("---The get entry---\r\n");
        BACKDOOR_MGR_Printf("mac: %02x-%02x-%02x-%02x-%02x-%02x\r\n",
        addr_entry.mac[0], addr_entry.mac[1], addr_entry.mac[2], addr_entry.mac[3], addr_entry.mac[4], addr_entry.mac[5]);
        BACKDOOR_MGR_Printf("vid: %d\r\n",addr_entry.vid);
        BACKDOOR_MGR_Printf("ifindex: %d\r\n",addr_entry.ifindex);
        BACKDOOR_MGR_Printf("action: %d\r\n",addr_entry.action);
        BACKDOOR_MGR_Printf("source: %d\r\n",addr_entry.source);
        BACKDOOR_MGR_Printf("life_time: %d\r\n",addr_entry.life_time);
    }
    else
        BACKDOOR_MGR_Printf("Get Address Entry Fail!!\r\n");
    return;
}

static void AMTR_MGR_BD_GetNextMVAddr(void)
{
 UI8_T   mac[12];
    UI32_T vid;
    UI8_T ch;
    UI8_T   index;
    AMTR_TYPE_AddrEntry_T addr_entry;

    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       Get Next Address Entry         ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    /* vlan
     */
    BACKDOOR_MGR_Printf("\r\n Set VID:");
    vid = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch > 9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        vid = vid*10+ch;
    }

    /* mac
     */
    BACKDOOR_MGR_Printf("\r\n MAC addr:");
    index = 0;
    while((index < 12) && ((ch=BACKDOOR_MGR_GetChar()) != 0xd))
    {
        if(ch >= '0' && ch <= '9')
        {
            ch -= '0';
        }
        else if(ch >= 'A' && ch <='F')
        {
            ch = ch - 'A' + 0x0a;
        }
        else if(ch >= 'a' && ch <='f')
        {
            ch = ch - 'a' + 0x0a;
        }
        else
        {
            BACKDOOR_MGR_Printf("Error in Input\r\n");
            return;
        }
        BACKDOOR_MGR_Printf("%x", ch);
        mac[index++] = ch;
    }

    for(index = 0; index < 6; index++)
    {
        mac[index] = (((mac[index*2] & 0x0F)<<4 ) | (mac[index*2 + 1] & 0x0F));
    }
    BACKDOOR_MGR_Printf("\r\nThe MAC you enter is %02x-%02x-%02x-%02x-%02x-%02x\r\n",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    addr_entry.vid=vid;
    amtrmgr_memcpy(addr_entry.mac, mac, AMTR_TYPE_MAC_LEN);
    if(AMTR_MGR_GetNextMVAddrEntry(&addr_entry,AMTR_MGR_GET_ALL_ADDRESS))
    {
        BACKDOOR_MGR_Printf("---The get entry---\r\n");
        BACKDOOR_MGR_Printf("mac: %02x-%02x-%02x-%02x-%02x-%02x\r\n",
        addr_entry.mac[0], addr_entry.mac[1], addr_entry.mac[2], addr_entry.mac[3], addr_entry.mac[4], addr_entry.mac[5]);
        BACKDOOR_MGR_Printf("vid: %d\r\n",addr_entry.vid);
        BACKDOOR_MGR_Printf("ifindex: %d\r\n",addr_entry.ifindex);
        BACKDOOR_MGR_Printf("action: %d\r\n",addr_entry.action);
        BACKDOOR_MGR_Printf("source: %d\r\n",addr_entry.source);
        BACKDOOR_MGR_Printf("life_time: %d\r\n",addr_entry.life_time);
    }
    else
        BACKDOOR_MGR_Printf("Get Next Address Entry Fail!!\r\n");
    return;
}

static void AMTR_MGR_BD_GetNextVMAddrEntry_ByLPortNMaskMatch(void)
{
    UI8_T  mac[12];
    UI32_T vid;
    UI32_T ifindex;
    UI8_T mac_list_num;
    UI8_T mac_list_ar[1][SYS_ADPT_MAC_ADDR_LEN];
    UI8_T mask_list_ar[1][SYS_ADPT_MAC_ADDR_LEN];
    AMTR_TYPE_AddrEntry_T addr_entry;
    UI32_T index;
    UI32_T i, v;
    UI8_T ch;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n        Get Next Address Entry Except Certain Addr        ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    /* vlan
     */
    BACKDOOR_MGR_Printf("\r\n Set VID:");
    vid = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch > 9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        vid = vid*10+ch;
    }

    /* ifindex
     */
    BACKDOOR_MGR_Printf("\r\n Set ifindex:");
    ifindex = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch>9)
        {
            BACKDOOR_MGR_Printf("\r\nError Input");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        ifindex = ifindex*10+ch;
    }

    /* mac
     */
    BACKDOOR_MGR_Printf("\r\n MAC addr:");
    index = 0;
    while((index < 12) && ((ch=BACKDOOR_MGR_GetChar()) != 0xd))
    {
        if(ch >= '0' && ch <= '9')
        {
            ch -= '0';
        }
        else if(ch >= 'A' && ch <='F')
        {
            ch = ch - 'A' + 0x0a;
        }
        else if(ch >= 'a' && ch <='f')
        {
            ch = ch - 'a' + 0x0a;
        }
        else
        {
            BACKDOOR_MGR_Printf("Error in Input\r\n");
            return;
        }
        BACKDOOR_MGR_Printf("%x", ch);
        mac[index++] = ch;
    }

    for(index = 0; index < 6; index++)
    {
        mac[index] = (((mac[index*2] & 0x0F)<<4 ) | (mac[index*2 + 1] & 0x0F));
    }
    BACKDOOR_MGR_Printf("\r\nThe MAC you enter is %02x-%02x-%02x-%02x-%02x-%02x\r\n",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    /* mac list
     */
    memset(mac_list_ar, 0, sizeof(mac_list_ar));
    memset(mask_list_ar, 0, sizeof(mask_list_ar));
    for (mac_list_num = 0; mac_list_num < sizeof(mac_list_ar)/SYS_ADPT_MAC_ADDR_LEN; mac_list_num++)
    {
        BOOL_T get_mask = FALSE;
        UI8_T *mac_p;
    get_mac_addr:
        if (get_mask)
        {
            BACKDOOR_MGR_Printf("\r\n Excluded MAC mask:");
            mac_p = mask_list_ar[mac_list_num];
        }
        else
        {
            BACKDOOR_MGR_Printf("\r\n Excluded MAC addr:");
            mac_p = mac_list_ar[mac_list_num];
        }
        for (i = 0, v = 0xff; i < 12; )
        {
            ch=BACKDOOR_MGR_GetChar();
            if (ch == 0xd && i == 0)
                goto get_mac_addr_finish;
            else if (ch == 0xd && i > 0)
                break;
            else if (isdigit(ch))
                v = ch - 0x30;
            else if (ch >= 'A' && ch <= 'F')
                v = ch - 'A' + 10;
            else if (ch >= 'a' && ch <= 'f')
                v = ch - 'a' + 10;
            if (v < 0x10)
            {
                BACKDOOR_MGR_Printf("%lX", v);
                if ((i & 1) == 1 && i != 11)
                    BACKDOOR_MGR_Printf("-");
                mac_p[i/2] |= v << ((i & 1)?0:4);
                i++;
                v = 0x10;
            }
        }
        if (i < 12)
        {
            BACKDOOR_MGR_Printf(" Error Input: ");
            goto get_mac_addr;
        }
        else if (!get_mask)
        {
            get_mask = TRUE;
            goto get_mac_addr;
        }
    }
get_mac_addr_finish:
    BACKDOOR_MGR_Printf("\r\n Number of MAC: %u", mac_list_num);

    addr_entry.vid=vid;
    addr_entry.ifindex=ifindex;
    amtrmgr_memcpy(addr_entry.mac, mac, AMTR_TYPE_MAC_LEN);
    if(AMTR_MGR_GetNextVMAddrEntryByLPortNMaskMatch(&addr_entry, mac_list_ar[0], mask_list_ar[0], AMTR_MGR_GET_ALL_ADDRESS))
    {
        BACKDOOR_MGR_Printf("---The get entry---\r\n");
        BACKDOOR_MGR_Printf("mac: %02x-%02x-%02x-%02x-%02x-%02x\r\n",
        addr_entry.mac[0], addr_entry.mac[1], addr_entry.mac[2], addr_entry.mac[3], addr_entry.mac[4], addr_entry.mac[5]);
        BACKDOOR_MGR_Printf("vid: %d\r\n",addr_entry.vid);
        BACKDOOR_MGR_Printf("ifindex: %d\r\n",addr_entry.ifindex);
        BACKDOOR_MGR_Printf("action: %d\r\n",addr_entry.action);
        BACKDOOR_MGR_Printf("source: %d\r\n",addr_entry.source);
        BACKDOOR_MGR_Printf("life_time: %d\r\n",addr_entry.life_time);
    }
    else
        BACKDOOR_MGR_Printf("\nGet Next Address Entry Fail!!\r\n");

    return;
}

static void AMTR_MGR_BD_SetAgingStatus()
{
    UI8_T ch;
    UI32_T status;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       Set Aging Status         ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n Status([1]enable,[2]disable):");
    ch=BACKDOOR_MGR_GetChar();
    ch -= 0x30;
    if(ch > 9)
    {
        BACKDOOR_MGR_Printf("Erro in Input\n");
        return;
    }
    BACKDOOR_MGR_Printf("%d", ch);
    status=ch;
    if(AMTR_MGR_SetAgingStatus(status))
        BACKDOOR_MGR_Printf("\r\nSet Aging Status success!!");
    else
        BACKDOOR_MGR_Printf("\r\nSet Aging Statu Fail!!\r\n");
    return;

}

static void AMTR_MGR_BD_SetAgingTime()
{
    UI8_T ch;
    UI32_T aging_time;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf(" \r\n      Set Aging Time         ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n Aging Time[10~1000000] (Sec.):  ");
    aging_time = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch > 9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        aging_time = aging_time*10+ch;
    }

    if(AMTR_MGR_SetDot1dTpAgingTime(aging_time))
        BACKDOOR_MGR_Printf("\r\nSet Aging Time success!!");
    else
        BACKDOOR_MGR_Printf("\r\nSet Aging Time Fail!!");
    return;

}

static void AMTR_MGR_BD_SetCpuMac(void)
{
    UI8_T   mac[12];
    UI32_T vid;
    UI8_T ch;
    UI8_T   index;

    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       Set CPU MAC         ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    /* vlan
     */
    BACKDOOR_MGR_Printf("\r\n Set VID:");
    vid = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch > 9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        vid = vid*10+ch;
    }
    /* mac
     */
    BACKDOOR_MGR_Printf("\r\n MAC addr:");
    index = 0;
    while((index < 12) && ((ch=BACKDOOR_MGR_GetChar()) != 0xd))
    {
        if(ch >= '0' && ch <= '9')
        {
            ch -= '0';
        }
        else if(ch >= 'A' && ch <='F')
        {
            ch = ch - 'A' + 0x0a;
        }
        else if(ch >= 'a' && ch <='f')
        {
            ch = ch - 'a' + 0x0a;
        }
        else
        {
            BACKDOOR_MGR_Printf("Error in Input\r\n");
            return;
        }
        BACKDOOR_MGR_Printf("%x", ch);
        mac[index++] = ch;
    }
    for(index = 0; index < 6; index++)
    {
        mac[index] = (((mac[index*2] & 0x0F)<<4 ) | (mac[index*2 + 1] & 0x0F));
    }

    BACKDOOR_MGR_Printf("\r\nThe MAC you enter is %02x-%02x-%02x-%02x-%02x-%02x\r\n",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    if(AMTR_MGR_SetInterventionEntry(vid,mac)==AMTR_TYPE_RET_SUCCESS)
        BACKDOOR_MGR_Printf("Set CPU MAC success!!\r\n");
    else
        BACKDOOR_MGR_Printf("Set CPU MAC Fail!!\r\n");
    return;
}

static void AMTR_MGR_BD_DeleteCpuMac(void)
{
    UI8_T   mac[12];
    UI32_T vid;
    UI8_T ch;
    UI8_T   index;

    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       Delete CPU MAC         ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    /* vlan
     */
    BACKDOOR_MGR_Printf("\r\n Set VID:");
    vid = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch > 9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        vid = vid*10+ch;
    }

    /* mac
     */
    BACKDOOR_MGR_Printf("\r\n MAC addr:");
    index = 0;
    while((index < 12) && ((ch=BACKDOOR_MGR_GetChar()) != 0xd))
    {
        if(ch >= '0' && ch <= '9')
        {
            ch -= '0';
        }
        else if(ch >= 'A' && ch <='F')
        {
            ch = ch - 'A' + 0x0a;
        }
        else if(ch >= 'a' && ch <='f')
        {
            ch = ch - 'a' + 0x0a;
        }
        else
        {
            BACKDOOR_MGR_Printf("Error in Input\r\n");
            return;
        }
        BACKDOOR_MGR_Printf("%x", ch);
        mac[index++] = ch;
    }

    for(index = 0; index < 6; index++)
    {
        mac[index] = (((mac[index*2] & 0x0F)<<4 ) | (mac[index*2 + 1] & 0x0F));
    }
    BACKDOOR_MGR_Printf("\r\nThe MAC you enter is %02x-%02x-%02x-%02x-%02x-%02x\r\n",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    if(AMTR_MGR_DeleteInterventionEntry(vid,mac))
        BACKDOOR_MGR_Printf("Delete CPU MAC success!!\r\n");
    else
        BACKDOOR_MGR_Printf("Delete CPU MAC Fail!!\r\n");

    return;
}

static void AMTR_MGR_BD_IsPortSecurityEnabled(void)
{
    UI32_T ifindex;
    UI8_T ch;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       Is Port Secuirty Enabled ??         ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n Set ifindex:");
    ifindex = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch>9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\r\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        ifindex = ifindex*10+ch;
    }
    if(AMTR_MGR_IsPortSecurityEnabled(ifindex))
        BACKDOOR_MGR_Printf("\r\nYes! Port Security is enabled!!");
    else
        BACKDOOR_MGR_Printf("\r\nNo! Port Security isn't enabled!!");

    return;
}

static void AMTR_MGR_BD_PerformanceTesting(void)
{
    UI32_T number;
    UI8_T ch;

    amtr_mgr_bd_perform_test=!amtr_mgr_bd_perform_test;
    if (amtr_mgr_bd_perform_test)
    {
        BACKDOOR_MGR_Printf("\r\n");
        BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        BACKDOOR_MGR_Printf("\r\n       Performance Testing         ");
        BACKDOOR_MGR_Printf("\r\n       ex. takes [X] ticks to Learn [Y] entries.         ");
        BACKDOOR_MGR_Printf("\r\n            INPUT: Y     OUTPUT:X         ");
        BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        BACKDOOR_MGR_Printf("\r\n Set number of entries[Max:0]:");
        number = 0;
        while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
        {
            ch -= 0x30;
            if(ch>9)
            {
                BACKDOOR_MGR_Printf("Erro in Input\r\n");
                return;
            }
            BACKDOOR_MGR_Printf("%d", ch);
            number = number*10+ch;
        }
        if (number > amtr_mgr_arl_capacity)
            BACKDOOR_MGR_Printf("\r\n Error Input!!");
        else if (number == 0)
            amtr_mgr_bd_test_amount = amtr_mgr_arl_capacity;
        else
            amtr_mgr_bd_test_amount = number;
        /* Start to test learning performance.
         */
        amtr_mgr_bd_start_learning_tick = 0;
    }
}

static void AMTR_MGR_BD_CombineEventTesting(void)
{
    UI8_T ch;
    UI32_T basis_field; /*1: ifindex 2:vid*/
    UI32_T number;
    UI32_T life_time;
    UI32_T source;

    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       Combine event sync to Hisam table          ");
    BACKDOOR_MGR_Printf("\r\n       Explain:  User have to input three parameter.          ");
    BACKDOOR_MGR_Printf("\r\n       1. basis field [1: ifindex  2:vid]          ");
    BACKDOOR_MGR_Printf("\r\n       2. Life Time          ");
    BACKDOOR_MGR_Printf("\r\n       3. Source          ");
    BACKDOOR_MGR_Printf("\r\n       Then backdoor function will test the combine events mechanism.          ");
    BACKDOOR_MGR_Printf("\r\n       ex. User input ifindex 1 + permanent + config.          ");
    BACKDOOR_MGR_Printf("\r\n            Backdoor function will call         ");
    BACKDOOR_MGR_Printf("\r\n            AMTR_MGR_DeleteAddrByLifeTimeAndLPort(1,permanent) and          ");
    BACKDOOR_MGR_Printf("\r\n            AMTR_MGR_DeleteAddrBySourceAndLPort(1, config) at the same time.          ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n Please choose a basis field[1: ifindex  2:vid]:");
    ch=BACKDOOR_MGR_GetChar();
    ch -= 0x30;
    if ((ch!=1)&&(ch!=2))
    {
        BACKDOOR_MGR_Printf("\r\nERROR INPUT!");
        return;
    }
    basis_field =ch;
    BACKDOOR_MGR_Printf("\r\n Please input the value of basis field(ifindex or vid):");
    number = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch>9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\r\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        number = number*10+ch;
    }
    /* life time
     */
    BACKDOOR_MGR_Printf("\r\nLife Time(1.Other 2.Invalid 3.Permanent 4.Reset 5.Timeout):");
    ch=BACKDOOR_MGR_GetChar();
    ch -= 0x30;
    life_time = ch;
    BACKDOOR_MGR_Printf("\r\nSource(1:other 2:invalid 3:learnt 4:self 5:config 6:security):");
    ch=BACKDOOR_MGR_GetChar();
    ch -= 0x30;
    source=ch;
    if (basis_field==1)
    {
        AMTR_MGR_DeleteAddrByLifeTimeAndLPort(number,life_time);
        AMTR_MGR_DeleteAddrBySourceAndLPort(number,source);
    }
    else
    {
        AMTR_MGR_DeleteAddrByVidAndLifeTime(number,life_time);
        AMTR_MGR_DeleteAddrByVidAndSource(number,source);
    }
}

static void AMTR_MGR_BD_ChangeArlCapacity(void)
{
    UI32_T capacity;
    UI8_T ch;
    UI32_T total_counter;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n       Change capacity of ARL table         ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n Set capacity [N] k (ARL table can learn [N] k entries):");
    capacity = 0;
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch>9)
        {
            BACKDOOR_MGR_Printf("Erro in Input\r\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        capacity = capacity*10+ch;
    }
    capacity *= 1024;
    if (capacity > SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY)
        BACKDOOR_MGR_Printf("\r\n ERROR INPUT!!");
    amtr_mgr_arl_capacity = capacity;
    total_counter = AMTRDRV_OM_GetTotalCounter();
    if (total_counter> amtr_mgr_arl_capacity)
    {
        AMTRDRV_MGR_DelNumOfDynamicAddrs(total_counter-amtr_mgr_arl_capacity);
    }

}
static void AMTR_MGR_BD_OpenReturnDebug(void)
{

    UI8_T ch;
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    BACKDOOR_MGR_Printf("\r\n     open return debug(0:disable 1:open)      ");
    BACKDOOR_MGR_Printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    while((ch=BACKDOOR_MGR_GetChar()) != 0xd)
    {
        ch -= 0x30;
        if(ch>1)
        {
            BACKDOOR_MGR_Printf("Erro in Input\r\n");
            return;
        }
        BACKDOOR_MGR_Printf("%d", ch);
    }
    if(1== ch || 0 == ch)
        amtr_mgr_debug_return = ch;
}
#if (AMTR_TYPE_DEBUG_PACKET_FLOW == 1)
static void AMTR_MGR_BD_OpenPacketFlowTraceForNA(void)
{
    I8_T ch;
    int index = 0;
    BACKDOOR_MGR_Printf("\r\namtr_mgr_na_interface_index %i\n",amtr_mgr_na_interface_index);
    BACKDOOR_MGR_Printf("\r\nThe packet flow trace is based on L2 ");
    BACKDOOR_MGR_Printf("interface(except L2 vlan),55 is all packet\n");
    while((ch = BACKDOOR_MGR_GetChar()) != 0xd){
        ch -= 0x30;
        if((atoi((char*)&ch) > 9)||(atoi((char*)&ch) < 0))  /*modified by Jinhua Wei, to remove warning,include type of variable 'ch'*/
        {
            BACKDOOR_MGR_Printf("Erro in Input\r\n");
            break;
        }
        BACKDOOR_MGR_Printf("%d", ch);
        index = index*10 + ch;
    }

    BACKDOOR_MGR_Printf("index :%d\n", index);

    if((index > 26)&&(index != 55)){
        BACKDOOR_MGR_Printf("Erro in Input\r\n");
        return;
    }

    amtr_mgr_na_interface_index = index;
}
#endif
#if(AMTR_TYPE_MEMCPY_MEMCOM_PERFORMANCE_TEST == 1)
void AMTR_MGR_GetMemActionCounter()
{
    char p;
    if ((p = BACKDOOR_MGR_GetChar())!=0xd)
    {
        if (p- '0' > 9 || p-'0' <1)
        {
            BACKDOOR_MGR_Printf("value %c is invalid\n",p);
            return ;
        }
        BACKDOOR_MGR_Printf("%c",p);
        if (p - '0' == 1)
        {
            BACKDOOR_MGR_Printf("reset the counter to 0\n");
            amtrmgr_memcpy_counter = 0;
            amtrmgr_memset_counter = 0;
            amtrmgr_memcmp_counter = 0;
        }
        if (p - '0' == 2)
        {
            BACKDOOR_MGR_Printf("the amtrmgr_memcpy_counter %ld,amtrmgr_memset_counter %ld,amtrmgr_memcmp_counter %ld \n",amtrmgr_memcpy_counter,amtrmgr_memset_counter,amtrmgr_memcmp_counter);
        }
    }
}

static UI8_T test_mac[SYS_ADPT_MAC_ADDR_LEN]= { 1,2,3,4,5,6 };
void AMTR_MGR_TestMemActionCost(){
    UI8_T  mac[SYS_ADPT_MAC_ADDR_LEN];
    int i = 0;
    int j = 0;
    int k = 0;
    UI32_T t1 =SYSFUN_GetSysTick();

    while(i++ < 44255)
        memcpy(mac,test_mac,SYS_ADPT_MAC_ADDR_LEN);

    while(j++ < 582)
        memset(security_port_move_buf,0,sizeof(security_port_move_buf));

    mac[5] = 7 ;

    while(k++ < 16593)
        memcmp(mac,test_mac,AMTR_TYPE_MAC_LEN);

    BACKDOOR_MGR_Printf("the start tick %ld, the ending tick %ld,%d,%d,%d\n",t1,SYSFUN_GetSysTick(),i,j,k);

}
#endif

static void AMTR_MGR_BackDoor_Menu (void)
{
    UI8_T   ch;
    BOOL_T  eof=FALSE;

    /*  BODY
     */
    tg_handle = L2_L4_PROC_COMM_GetNetaccessGroupTGHandle();

    /* Join thread group
     */
    if (L_THREADGRP_Join(tg_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY,
            &backdoor_member_id) == FALSE)
    {
        BACKDOOR_MGR_Printf("\n%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    /* Core layer CSC have no correct backdoor information in Slave mode.
     */
    if(amtr_mgr_operating_mode != SYS_TYPE_STACKING_MASTER_MODE)
    {
        BACKDOOR_MGR_Printf("\r\n AMTR_MGR doesn't support backdoor in this operating mode!");
        return;
    }

    while (!eof)
    {
        BACKDOOR_MGR_Printf("\r\n========================AMTR_MGR BackDoor Menu=========================");
        BACKDOOR_MGR_Printf("\r\n0. Exit");
        BACKDOOR_MGR_Printf("\r\n1. Dump Hisam Table                     2. Dump AMTR_Port_Info Table");
        BACKDOOR_MGR_Printf("\r\n3. Set AMTR_Port_Info Table             4. Set Address Entry");
        BACKDOOR_MGR_Printf("\r\n5. Delete Address Entry                 6. Delete By Port");
        BACKDOOR_MGR_Printf("\r\n7. Delete By Vid                        8. Delete By Life Time");
        BACKDOOR_MGR_Printf("\r\n9. Delete By Source                     a. Delete By Port+LifeTime");
        BACKDOOR_MGR_Printf("\r\nb. Delete By Port+Source                c. Delete By Vid+LifeTime");
        BACKDOOR_MGR_Printf("\r\nd. Delete By Vid+Source                 e. Delete By LifeTime+Vid+Port");
        BACKDOOR_MGR_Printf("\r\nf. Delete By Source+Vid+Port            g. Delete All Address");
        BACKDOOR_MGR_Printf("\r\nh. Get Exect Address Entry              i. Get Next MV Address Enry");
        BACKDOOR_MGR_Printf("\r\nj. Set Aging Status                     k. Set Aging Time");
        BACKDOOR_MGR_Printf("\r\nl. Set CPU MAC                          m. Delete CPU MAC");
        BACKDOOR_MGR_Printf("\r\nn. Is Port Security enabled ??");
        BACKDOOR_MGR_Printf("\r\no. Toggle: Printf Message when port move, intrusion, mac-notify, mlag");
        if (amtr_mgr_bd_print_msg)
            BACKDOOR_MGR_Printf("(ON):");
        else
            BACKDOOR_MGR_Printf("(OFF):");
        BACKDOOR_MGR_Printf("\r\np. Toggle: Start to performance testing");
        if (amtr_mgr_bd_perform_test)
            BACKDOOR_MGR_Printf("(ON):");
        else
            BACKDOOR_MGR_Printf("(OFF):");
        BACKDOOR_MGR_Printf("\r\nq. Combined events testing              r. ARL Table Capacity changed");
        BACKDOOR_MGR_Printf("\r\ns. open the return debug\n ");
#if (AMTR_TYPE_DEBUG_PACKET_FLOW == 1)
        BACKDOOR_MGR_Printf("\r\nt. open the NA packet flow trace \n ");
#endif
#if(AMTR_TYPE_MEMCPY_MEMCOM_PERFORMANCE_TEST == 1)
        BACKDOOR_MGR_Printf("x: 1 : reset the memset memcpy counter. 2: show the counter \n\r");
        BACKDOOR_MGR_Printf("y: 1 : reset the memset memcpy counter. 2: show the counter \n\r");
        BACKDOOR_MGR_Printf("z: calc the memcpy cost \n\r");
#endif
        BACKDOOR_MGR_Printf("A: Delete Address By Vid And Port Except Certain Addr \n\r");
        BACKDOOR_MGR_Printf("B: Get Next Address Entry Except Certain Addr \n\r");
        BACKDOOR_MGR_Printf("C: Set addrress entry with priority \n\r");
        BACKDOOR_MGR_Printf("\r\n=======================================================================");
        BACKDOOR_MGR_Printf("\r\n     select =");
        ch = BACKDOOR_MGR_GetChar();

        BACKDOOR_MGR_Printf("%c",ch);
        switch (ch)
        {
            case '0' :
                eof = TRUE;
                break;
            case '1':
                AMTR_MGR_BD_DumpHisam();
                break;
            case '2':
                AMTR_MGR_BD_DumpPortInfo();
                break;
            case '3':
                AMTR_MGR_BD_SetPortInfo();
                break;
            case '4':
                AMTR_MGR_BD_SetAddrEntry();
                break;
            case '5':
                AMTR_MGR_BD_DelAddrEntry();
                break;
            case '6':
                AMTR_MGR_BD_DelAddrByPort();
                break;
            case '7':
                AMTR_MGR_BD_DelAddrByVid();
                break;
            case '8':
                AMTR_MGR_BD_DelAddrByLifeTime();
                break;
            case '9':
                AMTR_MGR_BD_DelAddrBySource();
                break;
            case 'a':
                AMTR_MGR_BD_DelAddrByPortAndLifeTime();
                break;
            case 'b':
                AMTR_MGR_BD_DelAddrByPortAndSource();
                break;
            case 'c':
                AMTR_MGR_BD_DelAddrByVidAndLifeTime();
                break;
            case 'd':
                AMTR_MGR_BD_DelAddrByVidAndSource();
                break;
            case 'e':
                AMTR_MGR_BD_DelAddrByLifeTimenVidnPort();
                break;
            case 'f':
                AMTR_MGR_BD_DelAddrBySourcenVidnPort();
                break;
            case 'g':
                AMTR_MGR_BD_DelAllAddr();
                break;
            case 'h':
                AMTR_MGR_BD_GetAddr();
                break;
            case 'i':
                AMTR_MGR_BD_GetNextMVAddr();
                break;
            case 'j':
                AMTR_MGR_BD_SetAgingStatus();
                break;
            case 'k':
                AMTR_MGR_BD_SetAgingTime();
                break;
            case 'l':
                AMTR_MGR_BD_SetCpuMac();
                break;
            case 'm':
                AMTR_MGR_BD_DeleteCpuMac();
                break;
            case 'n':
                AMTR_MGR_BD_IsPortSecurityEnabled();
                break;
            case 'o':
                amtr_mgr_bd_print_msg =!amtr_mgr_bd_print_msg;
                break;
            case 'p':
                AMTR_MGR_BD_PerformanceTesting();
                break;
            case 'q':
                AMTR_MGR_BD_CombineEventTesting();
                break;
            case 'r':
                AMTR_MGR_BD_ChangeArlCapacity();
                break;
            case 's':
                AMTR_MGR_BD_OpenReturnDebug();
                break;
#if (AMTR_TYPE_DEBUG_PACKET_FLOW == 1)
            case 't':
                AMTR_MGR_BD_OpenPacketFlowTraceForNA();
                break;
#endif
#if(AMTR_TYPE_MEMCPY_MEMCOM_PERFORMANCE_TEST == 1)
            case 'x':
                AMTRDRV_MGR_GetMemActionCounter();
                break;
            case 'y':
                AMTR_MGR_GetMemActionCounter();
                break;
            case 'z':
                AMTR_MGR_TestMemActionCost();
                break;
#endif
            case 'A':
                AMTR_MGR_BD_DelAddrExceptByVidnPortCertainAddr();
                break;
            case 'B':
                AMTR_MGR_BD_GetNextVMAddrEntry_ByLPortNMaskMatch();
                break;
            case 'C':
                AMTR_MGR_BD_SetAddrEntryWithPriority();
                break;
            default :
                ch = 0;
                break;
        }
    }   /*  end of while    */

    L_THREADGRP_Leave(tg_handle, backdoor_member_id);
    return;
}   /*  AMTR_BackDoor_Menu   */
#endif/*AMTR_MGR_BACKDOOR_OPEN*/

#if (SYS_CPNT_AMTR_VLAN_MAC_LEARNING == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_MGR_SetVlanLearningDefaultStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Initialize vlan learning
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
static BOOL_T AMTR_MGR_SetVlanLearningDefaultStatus(void)
{
    UI32_T vid;

    for(vid = 1; vid <= SYS_ADPT_MAX_VLAN_ID; vid++)
    {
        LAN_SetVlanLearningStatus(vid, SYS_DFLT_AMTR_VLAN_MAC_LEARNING);
        if (TRUE == VLAN_OM_IsVlanExisted(vid))
        {
            SWDRV_SetVlanLearningStatus(vid, SYS_DFLT_AMTR_VLAN_MAC_LEARNING);
        }
    }

    return TRUE;
}
#endif

