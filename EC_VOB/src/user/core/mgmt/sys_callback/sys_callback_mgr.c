/* MODULE NAME:  sys_callback_mgr.c
 * PURPOSE:
 *  System callback manager is responsible for handling all callbacks across
 *  csc groups.
 *
 * NOTES:
 *  Compiler Control:
 *      UNIT_TEST  -  For doing unit test.
 *
 * HISTORY
 *    6/5/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_adpt.h"
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "l_mname.h"
#include "sys_callback_mgr.h"
#include "sys_callback_om.h"
#include "sys_callback_om_private.h"
#include "sysrsc_mgr.h"
#include "l_ipcmem.h"
#include "backdoor_mgr.h"
#include "amtr_type.h"
#include "amtrdrv_pom.h"
#include "swctrl_pom.h"
#include "amtr_pmgr.h"
#include "amtr_pom.h"
#include "lan.h"
#include "netcfg_type.h"
#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
#include "sys_callback_refined_om.h"
#endif
#if (SYS_CPNT_NDSNP == TRUE)
#include "ndsnp_pom.h"
#include "ndsnp_pmgr.h"
#endif
#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
#include "netcfg_pom_nd.h"
#endif
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
#include "cmgr.h"
#include "xstp_pom.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define IPV6_FORMAT        0x86DD

#define IPPROTO_ICMPV6      58          /* ICMPv6 */
#define IPPROTO_TCP          6          /* tcp                          */
#define IPPROTO_UDP         17          /* user datagram protocol       */
#define IPPROTO_NONE        59          /* IPv6 no next header          */

#define ICMPV6_NDISC_ROUTER_SOL     133 /* ICMP Router Solicitation     */
#define ICMPV6_NDISC_ROUTER_ADVT    134 /* ICMP Router Advertisement    */
#define ICMPV6_NDISC_NEIGH_SOL      135 /* ICMP Neighbor Solicitation   */
#define ICMPV6_NDISC_NEIGH_ADVT     136 /* ICMP Neighbor Advertisement  */
#define ICMPV6_NDISC_REDIRECT       137 /* ICMP Redirect                */

#define IPV6_EXT_HDR_HOP_BY_HOP     0   /* Hop-by-hop options header  */
#define IPV6_EXT_HDR_DESTINATION    60  /* Destination options header */
#define IPV6_EXT_HDR_ROUTING        43  /* Routing header             */
#define IPV6_EXT_HDR_FRAGMENT       44  /* Fragment header            */
#define IPV6_EXT_HDR_AUTHENTICATION 51  /* Authentication header      */
#define IPV6_EXT_HDR_SECURITY       50  /* Encapsulating security payload */

/* enum for L_MM ext trace id for L_MM_Malloc
 */
enum
{
    EXT_TRACE_ID_PORTMOVECALLBACK = 1,
    EXT_TRACE_ID_ANNOUNCENEWMACADDRESS,
    EXT_TRACE_ID_AGEOUTMACADDRESS,
    EXT_TRACE_ID_AUTHENTICATEPACKET_ASYNCRETURN,
    EXT_TRACE_ID_OF_PACKETIN_CALLBACK,
};

/* This value will be used while sending message through SYSFUN.
 * msg type must be larger than 0.
 */
#define SYS_CALLBACK_MGR_DEFAULT_MSG_TYPE 1

/* MACRO FUNCTION DECLARATIONS
 */
#define SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, callback_event_id) {\
            const UI32_T *msgq_list; \
            UI32_T count = 0; \
            msgq_list = SYS_CALLBACK_MGR_GetMsgqKeyList(callback_event_id); \
            while (msgq_list[count++] != 0){}; \
            count--; \
            if (!count) \
            { \
                L_MM_Mref_Release(&mref_handle_p); \
                return TRUE; \
            } \
            else if (count>1) \
            { \
                L_MM_Mref_AddRefCount(mref_handle_p, count-1); \
            } \
        }

#define SYS_CALLBACK_MGR_FUNC_BEG(sys_cbtype, cb_event_id)          \
    SYSFUN_Msg_T            *sysfun_msg_p;  \
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;   \
    sys_cbtype              *cbdata_msg_p;  \
    UI8_T                   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(sys_cbtype)))];   \
                                                                                                                \
    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;                                                                   \
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(sys_cbtype));                                  \
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;                                               \
    cbdata_msg_p= (sys_cbtype *)syscb_msg_p->callback_data;                                                     \
    syscb_msg_p->callback_event_id = cb_event_id;


/* DATA TYPE DECLARATIONS
 */

typedef struct Ipv6PktFormat_S
{
#if (SYS_HWCFG_LITTLE_ENDIAN_CPU==TRUE)
    UI32_T      flow_label:20,
                traffic_class:8,
                version:4;
#else
    UI32_T      version:4,
                traffic_class:8,
                flow_label:20;
#endif
    UI16_T      payload_len;
    UI8_T       next_hdr;
    UI8_T       hop_limit;

    UI8_T       src_addr[16];
    UI8_T       dst_addr[16];
    UI8_T       pay_load[0];
} __attribute__((packed, aligned(1)))Ipv6PktFormat_T;


/************************************************
 *  callback function pointer type definitions  *
 ************************************************
 */
/* AMTR */
typedef void (*SYS_CALLBACK_MGR_IntrusionMac_CBFun_T) (UI32_T src_lport, UI16_T vid, UI8_T *src_mac, UI8_T *dst_mac, UI16_T ether_type);
typedef void (*SYS_CALLBACK_MGR_PortMove_CBFun_T)(UI32_T num_of_entry, UI8_T *entry_buf_p);
typedef void (*SYS_CALLBACK_MGR_SecurityPortMove_CBFun_T)(UI32_T ifindex, UI32_T vid, UI8_T  *mac, UI32_T original_ifindex);
typedef void (*SYS_CALLBACK_MGR_MacNotify_CBFun_T)(UI32_T ifindex, UI16_T vid, UI8_T *mac_p, BOOL_T is_add);
typedef void (*SYS_CALLBACK_MGR_AutoLearn_CBFun_T)(UI32_T ifindex, UI32_T portsec_status);
typedef void (*SYS_CALLBACK_MGR_MACTableDeleteByPort_CBFun_T)(UI32_T ifindex, UI32_T reason);
typedef void (*SYS_CALLBACK_MGR_MACTableDeleteByVid_CBFun_T)(UI32_T vid);
typedef void (*SYS_CALLBACK_MGR_MACTableDeleteByVIDnPort_CBFun_T)(UI32_T vid, UI32_T ifindex);
typedef void (*SYS_CALLBACK_MGR_MacTableDeleteByLifeTime_CBFun_T)(UI32_T life_time);


/* VLAN */
#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
 typedef void (*SYS_CALLBACK_MGR_ProcessList_CBFun_T)(UI32_T subid,SYS_CALLBACK_MGR_REFINEList_CBData_T *entry);
#endif
typedef void (*SYS_CALLBACK_MGR_VlanCreate_CBFun_T)(UI32_T vid_ifindex, UI32_T vlan_status);
typedef void (*SYS_CALLBACK_MGR_VlanDestroy_CBFun_T)(UI32_T vid_ifindex, UI32_T vlan_status);
typedef void (*SYS_CALLBACK_MGR_VlanMemberAdd_CBFun_T)(UI32_T vid_ifindex, UI32_T lport_ifindex, UI32_T vlan_status);
typedef void (*SYS_CALLBACK_MGR_VlanMemberDelete_CBFun_T)(UI32_T vid_ifindex, UI32_T lport_ifindex, UI32_T vlan_status);
typedef void (*SYS_CALLBACK_MGR_VlanPortMode_CBFun_T)(UI32_T lport_ifindex, UI32_T vlan_port_mode);
typedef void (*SYS_CALLBACK_MGR_VlanFinishAddFirstTrunkMember_CBFun_T)(UI32_T trunk_ifindex, UI32_T member_ifindex);
typedef void (*SYS_CALLBACK_MGR_VlanFinishAddTrunkMember_CBFun_T)(UI32_T trunk_ifindex, UI32_T member_ifindex);
typedef void (*SYS_CALLBACK_MGR_VlanFinishDeleteTrunkMember_CBFun_T)(UI32_T trunk_ifindex, UI32_T member_ifindex);
typedef void (*SYS_CALLBACK_MGR_VlanFinishDeleteLastTrunkMember_CBFun_T)(UI32_T trunk_ifindex, UI32_T member_ifindex);
typedef void (*SYS_CALLBACK_MGR_VlanMemberDeleteByTrunk_CBFun_T)(UI32_T vid_ifindex, UI32_T lport_ifindex, UI32_T vlan_status);
typedef void (*SYS_CALLBACK_MGR_PvidChange_CBFun_T)(UI32_T lport_ifindex, UI32_T old_pvid, UI32_T new_pvid);
typedef void (*SYS_CALLBACK_MGR_IfOperStatusChanged_CBFun_T)(UI32_T vid_ifindex, UI32_T oper_status);
typedef void (*SYS_CALLBACK_MGR_VlanAddFirstTrunkMember_CBFun_T)(UI32_T dot1q_vlan_index, UI32_T trunk_ifindex, UI32_T member_ifindex);
typedef void (*SYS_CALLBACK_MGR_VlanAddTrunkMember_CBFun_T)(UI32_T dot1q_vlan_index, UI32_T trunk_ifindex, UI32_T member_ifindex);
typedef void (*SYS_CALLBACK_MGR_VlanDeleteTrunkMember_CBFun_T)(UI32_T dot1q_vlan_index, UI32_T trunk_ifindex, UI32_T member_ifindex);
typedef void (*SYS_CALLBACK_MGR_VlanDeleteLastTrunkMember_CBFun_T)(UI32_T dot1q_vlan_index, UI32_T trunk_ifindex, UI32_T member_ifindex);
typedef void (*SYS_CALLBACK_MGR_VlanNameChanged_CBFun_T)(UI32_T vid);
typedef void (*SYS_CALLBACK_MGR_ProtovlanGidBindingChanged_CBFun_T)(UI32_T lport);
typedef void (*SYS_CALLBACK_MGR_VlanMemberTagChanged_CBFun_T)(UI32_T vid_ifindex, UI32_T lport_ifindex);

/* XSTP */
typedef void (*SYS_CALLBACK_MGR_LportEnterForwarding_CBFun_T)(UI32_T xstid, UI32_T lport);
typedef void (*SYS_CALLBACK_MGR_LportLeaveForwarding_CBFun_T)(UI32_T xstid, UI32_T lport);
typedef void (*SYS_CALLBACK_MGR_LportChangeState_CBFun_T)(void);
/*add by Tony.Lei*/
typedef void (*SYS_CALLBACK_MGR_LportTcChange_CBFun_T)(BOOL_T is_mstp_mode,UI32_T xstid,UI32_T lport,BOOL_T is_root,UI32_T tc_timer/*,UI8_T *vlan_bit_map*/);

typedef void (*SYS_CALLBACK_MGR_StpChangeVersion_CBFun_T)(UI32_T mode, UI32_T status);

/* AMTRDRV */
typedef void (*SYS_CALLBACK_MGR_NewMacAddress_CBFun_T)(UI32_T number_of_entries, UI8_T *addr_buf);

/* NMTRDRV */
typedef void (*SYS_CALLBACK_MGR_UpdateLocalNmtrdrvStats_CBFun_T)(UI32_T update_type,UI32_T unit, UI32_T start_port, UI32_T port_amount);

/* SWCTRL */
typedef void (*SYS_CALLBACK_MGR_LPortType_CBFun_T)(UI32_T ifindex,UI32_T port_type);
typedef void (*SYS_CALLBACK_MGR_UPortType_CBFun_T)(UI32_T unit,UI32_T port,UI32_T port_type);
typedef void (*SYS_CALLBACK_MGR_TrunkMember_CBFun_T)(UI32_T trunk_ifindex, UI32_T member_ifindex);
typedef void (*SYS_CALLBACK_MGR_LPort_CBFun_T)(UI32_T ifindex);
typedef void (*SYS_CALLBACK_MGR_UPort_CBFun_T)(UI32_T unit,UI32_T port);
typedef void (*SYS_CALLBACK_MGR_LPortStatus_CBFun_T)(UI32_T ifindex, BOOL_T status, UI32_T reason_bmp);
typedef void (*SYS_CALLBACK_MGR_LPortSpeedDuplex_CBFun_T)( UI32_T ifindex,UI32_T speed_duplex);
typedef void (*SYS_CALLBACK_MGR_UPortSpeedDuplex_CBFun_T)(UI32_T unit,UI32_T port,UI32_T speed_duplex);
typedef void (*SYS_CALLBACK_MGR_uPortLacpEffectiveOperStatus_CBFun_T)(UI32_T unit,UI32_T port,UI32_T pre_status,UI32_T current_status);
typedef void (*SYS_CALLBACK_MGR_PortEffectiveOperStatus_CBFun_T)(UI32_T ifindex, UI32_T pre_status, UI32_T current_status, SWCTRL_OperDormantLevel_T level);
typedef void (*SYS_CALLBACK_MGR_IfMauChanged_CBFun_T)(UI32_T lport);
typedef void (*SYS_CALLBACK_MGR_PortLearningStatusChanged_CBFun_T)(UI32_T lport, BOOL_T learning);

/* SWDRV */
typedef void (*SYS_CALLBACK_MGR_SWDRV_PortLinkUp_CBFun_T)(UI32_T unit, UI32_T port);
typedef void (*SYS_CALLBACK_MGR_SWDRV_PortLinkDown_CBFun_T)(UI32_T unit, UI32_T port);
typedef void (*SYS_CALLBACK_MGR_SWDRV_CraftPortLinkUp_CBFun_T)(UI32_T unit);
typedef void (*SYS_CALLBACK_MGR_SWDRV_CraftPortLinkDown_CBFun_T)(UI32_T unit);
typedef void (*SYS_CALLBACK_MGR_SWDRV_PortTypeChanged_CBFun_T)(UI32_T unit, UI32_T port, UI32_T module_id, UI32_T port_type);
typedef void (*SYS_CALLBACK_MGR_SWDRV_PortSpeedDuplex_CBFun_T)(UI32_T unit, UI32_T port, UI32_T speed_duplex);
typedef void (*SYS_CALLBACK_MGR_SWDRV_PortFlowCtrl_CBFun_T)(UI32_T unit, UI32_T port, UI32_T flow_ctrl);
typedef void (*SYS_CALLBACK_MGR_SWDRV_HotSwapInsert_CBFun_T)(UI32_T unit, UI32_T port);
typedef void (*SYS_CALLBACK_MGR_SWDRV_HotSwapRemove_CBFun_T)(UI32_T unit, UI32_T port);
typedef void (*SYS_CALLBACK_MGR_SWDRV_PortSfpPresent_CBFun_T)(UI32_T unit, UI32_T port, BOOL_T is_present);
typedef void (*SYS_CALLBACK_MGR_SWDRV_PortSfpInfo_CBFun_T)(UI32_T unit, UI32_T port, SWCTRL_OM_SfpInfo_T *sfp_info_p);
typedef void (*SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfo_CBFun_T)(UI32_T unit, UI32_T port, SWCTRL_OM_SfpDdmInfo_T *sfp_ddm_info_p);
typedef void (*SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfoMeasured_CBFun_T)(UI32_T unit, UI32_T port, SWCTRL_OM_SfpDdmInfoMeasured_T *sfp_ddm_info_measured_p);

/* CFM */
#if (SYS_CPNT_CFM == TRUE)
typedef void (*SYS_CALLBACK_MGR_CFM_DefectNotify_CBFun_T)(UI16_T type, UI32_T mep_id, UI32_T lport, UI8_T level, UI16_T vid, BOOL_T defected);
#endif

/* IGMPSNP */
typedef void (*SYS_CALLBACK_MGR_IgmpSnoopStatusChanged_CBFun_T)(UI32_T igmpsnp_status);
typedef void (*SYS_CALLBACK_MGR_IgmpSnoopRouterPortChanged_CBFun_T)(UI32_T vid_ifidx, UI32_T lport_ifidx);
typedef void (*SYS_CALLBACK_MGR_IgmpSnoopGroupMemberChanged_CBFun_T)(UI32_T vid_ifidx, UI8_T *mip, UI32_T lport_ifidx);

/* SYSMGMT */
typedef void (*SYS_CALLBACK_MGR_PowerStatusChanged_CBFun_T)(UI32_T unit, UI32_T power, UI32_T status);
#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
typedef void (*SYS_CALLBACK_MGR_FanStatusChanged_CBFun_T)(UI32_T unit, UI32_T fan, UI32_T status);
#endif
#if (SYS_CPNT_THERMAL_DETECT == TRUE)
typedef void (*SYS_CALLBACK_MGR_ThermalStatusChanged_CBFun_T)(UI32_T unit, UI32_T thermal, UI32_T status);
#endif
typedef void (*SYS_CALLBACK_MGR_XFPModuleStatusChanged_CBFun_T)(UI32_T unit, UI32_T port, UI32_T status);

/* SYSDRV */
typedef void (*SYS_CALLBACK_MGR_SYSDRV_AlarmInputStatusChanged_CBFun_T)(UI32_T unit, UI32_T status);
typedef void (*SYS_CALLBACK_MGR_SYSDRV_AlarmOutputStatusChanged_CBFun_T)(UI32_T unit, UI32_T status);

typedef void (*SYS_CALLBACK_MGR_SYSDRV_PowerStatusChanged_CBFun_T)(UI32_T unit, UI32_T power, UI32_T status);
typedef void (*SYS_CALLBACK_MGR_SYSDRV_PowerTypeChanged_CBFun_T)(UI32_T unit, UI32_T power, UI32_T type);

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
typedef void (*SYS_CALLBACK_MGR_SYSDRV_FanStatusChanged_CBFun_T)(UI32_T unit, UI32_T fan, UI32_T status);
typedef void (*SYS_CALLBACK_MGR_SYSDRV_FanSpeedChanged_CBFun_T)(UI32_T unit, UI32_T fan, UI32_T speed);
#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
typedef void (*SYS_CALLBACK_MGR_SYSDRV_ThermalStatusChanged_CBFun_T)(UI32_T unit, UI32_T thermal, UI32_T status);
typedef void (*SYS_CALLBACK_MGR_SYSDRV_ThermalTemperatureChanged_CBFun_T)(UI32_T unit, UI32_T thermal, UI32_T status);
#endif

typedef void (*SYS_CALLBACK_MGR_SYSDRV_XFPModuleStatusChanged_CBFun_T)(UI32_T unit, UI32_T port, UI32_T status);
typedef void (*SYS_CALLBACK_MGR_SYSDRV_XenpakStatusChanged_CBFun_T)(UI32_T xenpak_type);

/* TRK */
typedef void (*SYS_CALLBACK_MGR_AddStaticTrunkMember_CBFun_T)(UI32_T trunk_ifindex, UI32_T tm_ifindex);
typedef void (*SYS_CALLBACK_MGR_DelStaticTrunkMember_CBFun_T)(UI32_T trunk_ifindex, UI32_T tm_ifindex);

/* LAN */
typedef void (*SYS_CALLBACK_MGR_LanReceivePacket_CBFun_T)(L_MM_Mref_Handle_T *mref_handle_p,
                                                           UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                           UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                           UI16_T  tag_info,
                                                           UI16_T  type,
                                                           UI32_T  pkt_length,
                                                           UI32_T  src_unit,
                                                           UI32_T  src_port,
                                                           UI32_T  packet_class);

typedef void (*SYS_CALLBACK_MGR_AuthenticatePacket_CBFun_T)(L_MM_Mref_Handle_T *mref_handle_p,
                                                            UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                            UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                            UI16_T  tag_info,
                                                            UI16_T  type,
                                                            UI32_T  pkt_length,
                                                            UI32_T  src_unit,
                                                            UI32_T  src_port,
                                                            SYS_CALLBACK_MGR_AuthenticatePacket_Result_T    auth_result,
                                                            void    *cookie);

/* ISC */
typedef void (*SYS_CALLBACK_MGR_IscReceiveStktplgPacket_CBFun_T)(ISC_Key_T  *isc_key_p,
                                                           L_MM_Mref_Handle_T *mref_handle_p,
                                                           UI32_T  rx_port);

/* L2_MUX */
typedef void (*SYS_CALLBACK_MGR_L2muxReceivePacket_CBFun_T)(L_MM_Mref_Handle_T *mref_handle_p,
                                                           UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                           UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                           UI16_T  tag_info,
                                                           UI16_T  type,
                                                           UI32_T  pkt_length,
                                                           UI32_T  lport);

typedef void (*SYS_CALLBACK_MGR_RxSnoopDhcpPacket_CBFun_T)(L_MM_Mref_Handle_T *mref_handle_p,
                                                           UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                           UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                           UI16_T  tag_info,
                                                           UI32_T  pkt_length,
                                                           UI32_T  lport,
                                                           BOOL_T  auth_result,
                                                           UI32_T  auth_csc,
                                                           void    *cookie);


/* IML */
typedef void (*SYS_CALLBACK_MGR_ImlReceivePacket_CBFun_T)(L_MM_Mref_Handle_T *mref_handle_p,
                                                           UI32_T  packet_length,
                                                           UI32_T  ifindex,
                                                           UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                           UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                           UI16_T  ingress_vid,
                                                           UI32_T  src_port);

typedef void (*SYS_CALLBACK_MGR_TxSnoopDhcpPacket_CBFun_T)(L_MM_Mref_Handle_T *mref_handle_p,
                                                           UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                           UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                           UI32_T  pkt_len,
                                                           UI32_T  egr_vidifindex,
                                                           UI32_T  egr_lport,
                                                           UI32_T  ing_lport);


/* MLDSNP */
typedef void (*SYS_CALLBACK_MGR_MldsnpReceivePacket_CBFun_T)(L_MM_Mref_Handle_T *mref_handle_p,
                                                           UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                           UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                           UI16_T  tag_info,
                                                           UI16_T  type,
                                                           UI32_T  pkt_length,
                                                           UI32_T  ip_ext_opt_len,
                                                           UI32_T  lport);

/* RA GUARD */
typedef void (*SYS_CALLBACK_MGR_RaGuardReceivePacket_CBFun_T)(L_MM_Mref_Handle_T *mref_handle_p,
                                                           UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                           UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                           UI16_T  ing_vid,
                                                           UI8_T   ing_cos,
                                                           UI8_T   pkt_type,
                                                           UI32_T  pkt_length,
                                                           UI32_T  src_lport);

/* DHCP6SNP */
typedef void (*SYS_CALLBACK_MGR_Dhcp6snpReceivePacket_CBFun_T)(L_MM_Mref_Handle_T *mref_handle_p,
                                                               UI32_T  packet_length,
                                                               UI32_T  ext_hdr_len,
                                                               UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                               UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                               UI16_T  ingress_vid,
                                                               UI32_T  src_port);


/* STKTPLG */
typedef void (*SYS_CALLBACK_MGR_StackState_CBFun_T)(UI32_T msg);
typedef void (*SYS_CALLBACK_MGR_ModuleStateChanged_CBFun_T)(UI32_T unit_id);

/* STKCTRL */
typedef void (*SYS_CALLBACK_MGR_SavingConfigStatus_CBFun_T)(UI32_T status);

/* NETCFG */
/* Triggered by IPCFG
 */
typedef void (*SYS_CALLBACK_MGR_RifCreated_CBFun_T)(UI32_T ifindex, L_INET_AddrIp_T *addr_p);
typedef void (*SYS_CALLBACK_MGR_RifActive_CBFun_T)(UI32_T ifindex, L_INET_AddrIp_T *addr_p);
typedef void (*SYS_CALLBACK_MGR_RifDown_CBFun_T)(UI32_T ifindex, L_INET_AddrIp_T *addr_p);
typedef void (*SYS_CALLBACK_MGR_RifDestroyed_CBFun_T)(UI32_T ifindex, L_INET_AddrIp_T *addr_p);
typedef void (*SYS_CALLBACK_MGR_NsmRouteChange_CBFun_T)(UI32_T address_family);
#if (SYS_CPNT_DHCPV6 == TRUE)
typedef void (*SYS_CALLBACK_MGR_NETCFG_IPv6AddrAutoConfigCBFun_T)(UI32_T ifindex, BOOL_T status);
typedef void (*SYS_CALLBACK_MGR_NETCFG_L3IfCreateCBFun_T)(UI32_T ifindex, BOOL_T status);
typedef void (*SYS_CALLBACK_MGR_NETCFG_L3IfDestroyCBFun_T)(UI32_T ifindex);
typedef void (*SYS_CALLBACK_MGR_NETCFG_L3IfOperStatusUpCBFun_T)(UI32_T ifindex, BOOL_T status);
typedef void (*SYS_CALLBACK_MGR_NETCFG_L3IfOperStatusDownCBFun_T)(UI32_T ifindex);
#endif

/* CLI */
typedef void (*SYS_CALLBACK_MGR_ProvisionComplete_CBFun_T) (void);
typedef void (*SYS_CALLBACK_MGR_ModuleProvisionComplete_CBFun_T)(void);
typedef void (*SYS_CALLBACK_MGR_EnterTransitionMode_CBFun_T) (void);

/* LLDP */
typedef void (*SYS_CALLBACK_MGR_TelephoneDetect_CBFun_T)(UI32_T lport,
                                                         UI8_T *mac_addr,
                                                         UI8_T network_addr_subtype,
                                                         UI8_T *network_addr,
                                                         UI8_T network_addr_len,
                                                         UI32_T network_addr_ifindex,
                                                         BOOL_T tel_exist);

#ifdef SYS_CPNT_POE_PSE_DOT3AT
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
typedef void (*SYS_CALLBACK_MGR_Dot3atInfoReceived_CBFun_T)(UI32_T   unit,
                                                           UI32_T   port,
                                                           UI8_T    power_type,
                                                           UI8_T    power_source,
                                                           UI8_T    power_priority,
                                                           UI16_T   power_value,
                                                           UI8_T    requested_power_type,
                                                           UI8_T    requested_power_source,
                                                           UI8_T    requested_power_priority,
                                                           UI16_T   requested_power_value,
                                                           UI8_T    acknowledge);
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
typedef void (*SYS_CALLBACK_MGR_Dot3atInfoReceived_CBFun_T)(UI32_T   unit,
                                                           UI32_T   port,
                                                           UI8_T    power_type,
                                                           UI8_T    power_source,
                                                           UI8_T    power_priority,
                                                           UI16_T   pd_requested_power,
                                                           UI16_T   pse_allocated_power);
#endif
#endif

#if (SYS_CPNT_CN == TRUE)
typedef void (*SYS_CALLBACK_MGR_CnRemoteChange_CBFun_T) (UI32_T lport,
    UI32_T neighbor_num, UI8_T cnpv_indicators, UI8_T ready_indicators);
#endif

/* PRIMGMT */
typedef void (*SYS_CALLBACK_MGR_CosChanged_CBFun_T)(UI32_T lport_ifindex);

/* RADIUS */
typedef void (*SYS_CALLBACK_MGR_AnnounceRADIUSPacket_CBFun_T) (
            UI32_T  result,     UI8_T   *data_buf,  UI32_T  data_len,
            UI32_T  src_port,   UI8_T   *src_mac,   UI32_T  src_vid,
            char  *authorized_vlan_list,  char   *authorized_qos_list,
            UI32_T  session_timeout,        UI32_T  server_ip);

#if (SYS_CPNT_IGMPAUTH == TRUE)
typedef void (*SYS_CALLBACK_MGR_AnnounceRadiusIGMPAuthResult_CBFun_T)(
    UI32_T   result,
    UI32_T   auth_port,
    UI32_T   ip_address,
    UI8_T    *auth_mac,
    UI32_T   vlan_id,
    UI32_T   src_ip,
    UI8_T    msg_type);
#endif /* #if (SYS_CPNT_IGMPAUTH == TRUE) */

typedef void (*SYS_CALLBACK_MGR_AnnounceRadaAuthRes_CBFun_T)  (
            UI32_T  lport,      UI8_T   *mac,       int     identifier,
            BOOL_T  authorized_result,      char   *authorized_vlan_list,
            char   *authorized_qos_list,   UI32_T  session_time,   UI32_T server_ip);

/*maggie liu for RADIUS authentication ansync*/
typedef void (*SYS_CALLBACK_MGR_AnnounceRadiusAuthRes_CBFun_T)  (UI32_T result, I32_T privilege, UI32_T msg_type);

typedef void (*SYS_CALLBACK_MGR_AnnounceRadiusAuthRes_2_CBFun_T)(
    I32_T result,
    UI32_T privilege,
    void *cookie,
    UI32_T cookie_size
);

/* DOT1X */
typedef void (*SYS_CALLBACK_MGR_AnnounceDot1xAuthRes_CBFun_T) (
            UI32_T  lport,      UI8_T   *mac,       int     identifier,
            BOOL_T  authorized_result,      char   *authorized_vlan_list,
            char   *authorized_qos_list,   UI32_T  session_time,   UI32_T server_ip);

/* XFER */
typedef void (*SYS_CALLBACK_MGR_AnnounceXferResult_CBFun_T)(UI32_T cookie, void *arg_cookie, UI32_T arg_status);

/* HTTP SSHD */
typedef void (*SYS_CALLBACK_MGR_AnnounceCliXferResult_CBFun_T)(UI32_T cookie, void *arg_cookie, UI32_T arg_status);

/* AMTRL3 */
#if (SYS_CPNT_AMTRL3 == TRUE)
//typedef void (*SYS_CALLBACK_MGR_Nexthop_Status_Change_CBFun_T)(UI32_T action_flags, UI32_T fib_id, UI32_T status, IpAddr_T ip_addr, UI32_T lport_ifindex, UI32_T vid_ifindex, UI8_T *dst_mac);
#endif
#if (SYS_CPNT_DHCP == TRUE)
typedef void (*SYS_CALLBACK_MGR_DHCP_Restart3_CBFun_T) (UI32_T restart_object);
typedef void (*SYS_CALLBACK_MGR_DHCP_SetIfRole_CBFun_T) (UI32_T vid_ifindex, UI32_T role);
typedef void (*SYS_CALLBACK_MGR_DHCP_SetIfStatus_CBFun_T) (UI32_T vid_ifindex, UI32_T status);
#endif
#if (SYS_CPNT_CLI_DYNAMIC_PROVISION_VIA_DHCP == TRUE)
    typedef void (*SYS_CALLBACK_MGR_DHCP_RxOptionConfig_CBFun_T) (UI32_T  option66_length_p,
                                                                             UI8_T   *option66_data_p,
                                                                             UI32_T  option67_length_p,
                                                                             char    *option67_data_p);
#endif

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
    typedef void (*SYS_CALLBACK_MGR_AnnouceReloadRemainDate_CBFun_T)(UI32_T remaining_minutes);
#endif

#if (SYS_CPNT_IP_TUNNEL == TRUE)
    typedef void (*SYS_CALLBACK_MGR_AnnouceIpv6Packet_CBFun_T)(UI8_T* src_addr, UI8_T* dest_addr);
    typedef void (*SYS_CALLBACK_MGR_TunnelNetRouteHitBitChange_CBFun_T)(UI32_T fib_id, UI8_T* dst_addr, UI32_T preflen, UI32_T unit_id);
#endif

#if (SYS_CPNT_POE == TRUE)
typedef void (*SYS_CALLBACK_MGR_POEDRV_PortDetectionStatusChange_CBFun_T) (UI32_T unit, UI32_T port, UI32_T status);
typedef void (*SYS_CALLBACK_MGR_POEDRV_PortStatusChange_CBFun_T) (UI32_T unit, UI32_T port, UI32_T status);
typedef void (*SYS_CALLBACK_MGR_POEDRV_IsMainPowerReachMaximum_CBFun_T) (UI32_T unit, UI32_T status);
typedef void (*SYS_CALLBACK_MGR_POEDRV_PortOverloadStatusChange_CBFun_T) (UI32_T unit, UI32_T port, UI32_T status);
typedef void (*SYS_CALLBACK_MGR_POEDRV_PortPowerConsumptionChange_CBFun_T) (UI32_T unit, UI32_T port, UI32_T power);
typedef void (*SYS_CALLBACK_MGR_POEDRV_PortPowerClassificationChange_CBFun_T) (UI32_T unit, UI32_T port, UI32_T classification);
typedef void (*SYS_CALLBACK_MGR_POEDRV_MainPseConsumptionChange_CBFun_T) (UI32_T unit, UI32_T power);
typedef void (*SYS_CALLBACK_MGR_POEDRV_PseOperStatusChange_CBFun_T) (UI32_T unit, UI32_T status);
typedef void (*SYS_CALLBACK_MGR_POEDRV_PowerDeniedOccurFrequently_CBFun_T) (UI32_T unit, UI32_T port);
typedef void (*SYS_CALLBACK_MGR_POEDRV_PortFailureStatusChange_CBFun_T) (UI32_T unit, UI32_T port, UI32_T status);
#endif

#if (SYS_CPNT_CLUSTER == TRUE)
    typedef void (*SYS_CALLBACK_MGR_CLUSTER_ChangeRole_CBFun_T)(UI32_T role);
#endif

typedef void (*SYS_CALLBACK_MGR_AnnounceAclDeleted_CBFun_T)(char* acl_name, UI32_T acl_type, UI8_T *port_list);
typedef void (*SYS_CALLBACK_MGR_AnnouncePolicyMapDeleted_CBFun_T)(char* policy_map_name, UI8_T *port_list);

#if (SYS_CPNT_COS == TRUE)
typedef void (*SYS_CALLBACK_MGR_AnnounceCosPortConfigChanged_CBFun_T)(UI32_T l_port, UI32_T priority_of_config);
#endif /* #if (SYS_CPNT_COS == TRUE) */

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
typedef void (*SYS_CALLBACK_MGR_AnnounceMgmtIPFltChanged_CBFun_T)(UI32_T mode);
#endif /* #if (SYS_CPNT_MGMT_IP_FLT == TRUE) */

typedef struct SYS_CALLBACK_MGR_Icmp6Hdr_S {
    UI8_T       icmp6_type;
    UI8_T       icmp6_code;
    UI16_T      icmp6_cksum;
} __attribute__((packed, aligned(1)))SYS_CALLBACK_MGR_Icmp6Hdr_T;


#if (SYS_CPNT_DCBX == TRUE)
typedef void (*SYS_CALLBACK_MGR_EtsPfcCfgChanged_CBFun_T)(UI32_T lport);
typedef void (*SYS_CALLBACK_MGR_EtsReceived_CBFun_T)( UI32_T   lport,
                                                    BOOL_T  is_delete,
                                                    BOOL_T  rem_recommend_rcvd,
                                                    BOOL_T  rem_willing,
                                                    BOOL_T  rem_cbs,
                                                    UI8_T  rem_max_tc,
                                                    UI8_T  *rem_config_pri_assign_table,
                                                    UI8_T   *rem_config_tc_bandwidth_table,
                                                    UI8_T   *rem_config_tsa_assign_table,
                                                    UI8_T  *rem_recommend_pri_assign_table,
                                                    UI8_T   *rem_recommend_tc_bandwidth_table,
                                                    UI8_T   *rem_recommend_tsa_assign_table);

typedef void (*SYS_CALLBACK_MGR_PfcReceived_CBFun_T)(UI32_T   lport,
                                                    BOOL_T  is_delete,
                                                     UI8_T   *rem_mac,
                                                    BOOL_T  rem_willing,
                                                    BOOL_T  rem_mbc,
                                                    UI8_T  rem_pfc_cap,
                                                    UI8_T  rem_pfc_enable);
#endif  /* #if (SYS_CPNT_DCBX == TRUE) */

#if (SYS_CPNT_PBR == TRUE)
typedef void (*SYS_CALLBACK_MGR_HostRouteChanged_CBFun_T)(L_INET_AddrIp_T *addr_p, BOOL_T is_unresolved);
typedef void (*SYS_CALLBACK_MGR_AclChanged_CBFun_T)(UI32_T acl_index, char *acl_name, UI8_T type);
typedef void (*SYS_CALLBACK_MGR_RouteMapChanged_CBFun_T)(char *rmap_name, UI32_T seq_num, BOOL_T is_deleted);

#endif

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T SYS_CALLBACK_MGR_SendMsg(UI32_T src_csc_id, UI32_T callback_event_id, SYSFUN_Msg_T *sysfun_msg_p, BOOL_T is_notify_ipcfail);
static BOOL_T SYS_CALLBACK_MGR_SendMsgWithMsgQKey(UI32_T src_csc_id, UI32_T dest_msgq_key, SYSFUN_Msg_T *sysfun_msg_p, BOOL_T is_notify_ipcfail);
static BOOL_T SYS_CALLBACK_MGR_SendMsgWithFailCount(UI32_T src_csc_id, UI32_T callback_event_id, SYSFUN_Msg_T *sysfun_msg_p,
                                                    BOOL_T is_notify_ipcfail, UI32_T *fail_count_p);
static const UI32_T* SYS_CALLBACK_MGR_GetMsgqKeyList(UI32_T callback_event_id);
#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
static const BOOL_T SYS_CALLBACK_IsSwitchGroupEventId(UI32_T callback_event_id);
static const BOOL_T SYS_CALLBACK_MGR_CheckRefineMsgqList(UI32_T callback_event_id);
static void SYS_CALLBACK_MGR_GetPosition(UI32_T index, UI32_T *list_index, UI32_T *list_position);
static const BOOL_T SYS_CALLBACK_MGR_GenVlanList(UI32_T callback_event_id,
                                                                         UI32_T src_csc_id,
                                                                         BOOL_T is_notify_ipcfail,
                                                                         UI8_T*  callback_data);
static const BOOL_T SYS_CALLBACK_MGR_IslistMsgqList(UI32_T callback_event_id);
static BOOL_T SYS_CALLBACK_MGR_GetSubCallbackEventId(SYSFUN_Msg_T *sysfun_msg_p,UI32_T *sub_callback_event_id);
static const BOOL_T SYS_CALLBACK_MGR_IsVlanlistEventId(UI32_T callback_event_id);
static const BOOL_T SYS_CALLBACK_MGR_IsPortlistEventId(UI32_T callback_event_id);
#endif
#if (SYS_CPNT_SYS_CALLBACK_SOCKET == TRUE)
typedef struct SYS_CALLBACK_MGR_Sock_S
{
    int    socketfd;
    char   *sock_filename;
} SYS_CALLBACK_MGR_Sock_T;

static SYS_CALLBACK_MGR_Sock_T* SYS_CALLBACK_MGR_GetSocketList(UI32_T callback_event_id);
#endif
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
static BOOL_T SYS_CALLBACK_MGR_NotifyCmgrGroup();
#endif

/* STATIC VARIABLE DECLARATIONS
 */

#if (SYS_CPNT_SYS_CALLBACK_SOCKET == TRUE)
/*********************************************************
 *  definitions of Unix domain socket for each callback event  *
 *********************************************************
 */
/* SWDRV */
static SYS_CALLBACK_MGR_Sock_T uport_link_updown_socket_list[] =
{
    {-1, SYS_BLD_SYS_CALLBACK_SOCKET_CLIENT_OF_EVT} /* SYS_MODULE_SWDRV */,
    {-1, NULL}

};
/* L2MUX */
static SYS_CALLBACK_MGR_Sock_T l2mux_receive_of_packetin_socket_list[] =
{
    {-1, SYS_BLD_SYS_CALLBACK_SOCKET_CLIENT_OF_PACKETIN} /* SYS_MODULE_L2MUX */,
    {-1, NULL}
};


#endif
/*********************************************************
 *  definitions of msgqkey_list for each callback event  *
 *********************************************************
 */
/* AMTR */
static const UI32_T intrusionMac_msgqkey_list[] =
{
    0
};

static const UI32_T port_move_msgqkey_list[] =
{
#if (SYS_CPNT_AMTRL3 == TRUE)
    SYS_BLD_AMTRL3_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T securityPortMove_msgqkey_list[] =
{
#ifdef UNIT_TEST
    SYS_BLD_XXX_PROC_CSCGROUP1_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T auto_learn_msgqkey_list[] =
{
    0
};

static const UI32_T mac_table_delete_by_port_msgqkey_list[] =
{
#if (SYS_CPNT_AMTRL3 == TRUE)
    SYS_BLD_AMTRL3_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T mac_table_delete_by_vid_msgqkey_list[] =
{
    0
};

static const UI32_T mac_table_delete_by_vid_and_port_msgqkey_list[] =
{
#if (SYS_CPNT_AMTRL3 == TRUE)
    SYS_BLD_AMTRL3_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T mac_table_delete_by_life_time_msgqkey_list[] =
{
#if (SYS_CPNT_AMTRL3 == TRUE)
    SYS_BLD_AMTRL3_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T mac_addr_update_msgqkey_list[] =
{
#if (SYS_CPNT_AMTRL3 == TRUE)
    SYS_BLD_AMTRL3_GROUP_IPCMSGQ_KEY,
#endif
    0
};

/* VLAN */
static const UI32_T vlan_create_msgqkey_list[] =
{
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_L2MCAST == TRUE)
    SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_CFM==TRUE)
    SYS_BLD_CFM_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_VXLAN==TRUE)
    SYS_BLD_VXLAN_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T vlan_create_for_gvrp_msgqkey_list[] =
{
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T vlan_destroy_msgqkey_list[] =
{
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_L2MCAST == TRUE)
    SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_CFM==TRUE)
    SYS_BLD_CFM_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_VXLAN==TRUE)
    SYS_BLD_VXLAN_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T vlan_destroy_for_gvrp_msgqkey_list[] =
{
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T L3_vlan_destroy_msgqkey_list[] =
{
    SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_AMTRL3 == TRUE)
    SYS_BLD_AMTRL3_GROUP_IPCMSGQ_KEY,
#endif /* SYS_CPNT_AMTRL3 */
    0
};

static const UI32_T vlan_member_add_msgqkey_list[] =
{
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_L2MCAST == TRUE)
    SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY,
#endif
    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_CFM==TRUE)
    SYS_BLD_CFM_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_VXLAN==TRUE)
    SYS_BLD_VXLAN_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T vlan_member_add_for_gvrp_msgqkey_list[] =
{
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T vlan_member_delete_msgqkey_list[] =
{
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_L2MCAST == TRUE)
    SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY,
#endif
 #if (SYS_CPNT_CFM==TRUE)
    SYS_BLD_CFM_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_VXLAN==TRUE)
    SYS_BLD_VXLAN_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T vlan_member_delete_for_gvrp_msgqkey_list[] =
{
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T vlan_port_mode_msgqkey_list[] =
{
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_L2MCAST == TRUE)
    SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T finish_add_first_trunk_member_msgqkey_list[] =
{
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
/*move to trunk_member_add_1st_msgqkey_list
#if (SYS_CPNT_AMTRL3 == TRUE)
    SYS_BLD_AMTRL3_GROUP_IPCMSGQ_KEY,
#endif
    SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY,*/
    0
};

static const UI32_T finish_add_trunk_member_msgqkey_list[] =
{
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
/*move to trunk_member_add_msgqkey_list
#if (SYS_CPNT_AMTRL3 == TRUE)
    SYS_BLD_AMTRL3_GROUP_IPCMSGQ_KEY,
#endif
    SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY,*/
    0
};

static const UI32_T finish_delete_trunk_member_msgqkey_list[] =
{
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
/*move to trunk_member_delete_msgqkey_list
#if (SYS_CPNT_AMTRL3 == TRUE)
    SYS_BLD_AMTRL3_GROUP_IPCMSGQ_KEY,
#endif
   SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY,*/
    0
};

static const UI32_T finish_delete_last_trunk_member_msgqkey_list[] =
{
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
/*move to trunk_member_delete_lst_msgqkey_list
#if (SYS_CPNT_AMTRL3 == TRUE)
    SYS_BLD_AMTRL3_GROUP_IPCMSGQ_KEY,
#endif
    SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY,*/
    0
};

static const UI32_T vlan_member_delete_by_trunk_msgqkey_list[] =
{
#if 0
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,/* because this port already jion trunk port, this port becaome inactive for gvrp*/
    SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY,/* it needn't because trunk member add will handle it?*/
#if (SYS_CPNT_AMTRL3 == TRUE)
    SYS_BLD_AMTRL3_GROUP_IPCMSGQ_KEY,
#endif
#endif
    0
};

static const UI32_T pvid_change_msgqkey_list[] =
{
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T l3if_oper_status_changed_msgqkey_list[] =
{
    SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_AMTRL3 == TRUE)
    SYS_BLD_AMTRL3_GROUP_IPCMSGQ_KEY,
#endif /* SYS_CPNT_AMTRL3 */
    0
};
static const UI32_T add_first_trunk_member_msgqkey_list[] =
{
    0
};

static const UI32_T add_trunk_member_msgqkey_list[] =
{
    0
};

static const UI32_T delete_trunk_member_msgqkey_list[] =
{
    0
};

static const UI32_T delete_last_trunk_member_msgqkey_list[] =
{
    0
};

static const UI32_T vlan_name_changed_msgqkey_list[] =
{
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T protovlan_gid_binding_changed_msgqkey_list[] =
{
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T vlan_member_tag_changed_msgqkey_list[] =
{
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
    0
};

/* XSTP */
static const UI32_T lport_enter_forwarding_msgqkey_list[] =
{
#if (SYS_CPNT_CFM==TRUE)
    /*it better to put cfm at first*/
    SYS_BLD_CFM_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_L2MCAST == TRUE)
    SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_NTP == TRUE)
    SYS_BLD_APP_PROTOCOL_GROUP_IPCMSGQ_KEY,
#endif /* #if (SYS_CPNT_NTP == TRUE) */
    0
};

static const UI32_T lport_leave_forwarding_msgqkey_list[] =
{
#if (SYS_CPNT_CFM==TRUE)
    /*it better to put cfm at first*/
    SYS_BLD_CFM_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_L2MCAST == TRUE)
    SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T lport_change_state_msgqkey_list[] =
{
    0
};
/*add by Tony.Lei for IGMPSnooping*/
static const UI32_T lport_tc_change_msgqkey_list[] =
{
#if (SYS_CPNT_L2MCAST == TRUE)
    SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY,
#endif
    0
};
static const UI32_T stp_change_version_msgqkey_list[] =
{
    0
};

/* the L3 just care agingout MAC entries , so new a agingout message from newmac message*/
static const UI32_T announce_new_mac_address_msgqkey_list[] =
{
    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
    0
};
/* the L3 just care agingout MAC entries , so new a agingout message from newmac message*/
static const UI32_T announce_agingout_mac_address_msgqkey_list[] =
{
#if (SYS_CPNT_AMTR==TRUE) && (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
#endif
    0
};

/* nmtrdrv */
static const UI32_T update_local_nmtrdrv_stats_msgqkey_list[] =
{
    SYS_BLD_NETACCESS_NMTR_IPCMSGQ_KEY,//SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
    0
};

/* SWCTRL */
static const UI32_T lport_type_changed_msgqkey_list[] =
{
    0
};

static const UI32_T uport_type_changed_msgqkey_list[] =
{
    0
};

static const UI32_T trunk_member_add_1st_msgqkey_list[] =
{
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_L2MCAST == TRUE)
    SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY,
#endif
    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
    SYS_BLD_L4_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_AMTRL3 == TRUE)
    SYS_BLD_AMTRL3_GROUP_IPCMSGQ_KEY,
#endif /* SYS_CPNT_AMTRL3 */
#if (SYS_CPNT_CFM==TRUE)
    SYS_BLD_CFM_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_PPPOE_IA == TRUE)
    SYS_BLD_PPPOE_IA_GROUP_IPCMSGQ_KEY,
#endif
    /*xiedan add for lldp sync the trunk and port config*/
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
    SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY,
#endif
#if ((SYS_CPNT_DHCPV6SNP == TRUE)||(SYS_CPNT_NDSNP == TRUE)||(SYS_CPNT_DHCPSNP == TRUE) || (SYS_CPNT_IPV6_SOURCE_GUARD == TRUE))
    SYS_BLD_DHCPSNP_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_DCB_GROUP == TRUE)
    SYS_BLD_DCB_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_VXLAN == TRUE)
    SYS_BLD_VXLAN_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T trunk_member_add_msgqkey_list[] =
{
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_L2MCAST == TRUE)
    SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY,
#endif
    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
    SYS_BLD_L4_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_AMTRL3 == TRUE)
    SYS_BLD_AMTRL3_GROUP_IPCMSGQ_KEY,
#endif /* SYS_CPNT_AMTRL3 */
#if (SYS_CPNT_CFM==TRUE)
    SYS_BLD_CFM_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_PPPOE_IA == TRUE)
    SYS_BLD_PPPOE_IA_GROUP_IPCMSGQ_KEY,
#endif
    /*xiedan add for lldp sync the trunk and port config*/
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
    SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY,
#endif
#if ((SYS_CPNT_DHCPV6SNP == TRUE)||(SYS_CPNT_NDSNP == TRUE)||(SYS_CPNT_DHCPSNP == TRUE)||(SYS_CPNT_IPV6_SOURCE_GUARD == TRUE))
    SYS_BLD_DHCPSNP_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_DCB_GROUP == TRUE)
    SYS_BLD_DCB_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_VXLAN == TRUE)
    SYS_BLD_VXLAN_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T trunk_member_delete_msgqkey_list[] =
{
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_L2MCAST == TRUE)
    SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY,
#endif
    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
    SYS_BLD_L4_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_AMTRL3 == TRUE)
    SYS_BLD_AMTRL3_GROUP_IPCMSGQ_KEY,
#endif /* SYS_CPNT_AMTRL3 */
#if (SYS_CPNT_CFM==TRUE)
    SYS_BLD_CFM_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_PPPOE_IA == TRUE)
    SYS_BLD_PPPOE_IA_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
    SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY,
#endif
#if ((SYS_CPNT_DHCPV6SNP == TRUE)||(SYS_CPNT_NDSNP == TRUE)||(SYS_CPNT_DHCPSNP == TRUE) || (SYS_CPNT_IPV6_SOURCE_GUARD == TRUE))
    SYS_BLD_DHCPSNP_GROUP_IPCMSGQ_KEY,
#endif
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_DCB_GROUP == TRUE)
    SYS_BLD_DCB_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_VXLAN == TRUE)
    SYS_BLD_VXLAN_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T trunk_member_delete_lst_msgqkey_list[] =
{
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_L2MCAST == TRUE)
    SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY,
#endif
    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_AMTRL3 == TRUE)
    SYS_BLD_AMTRL3_GROUP_IPCMSGQ_KEY,
#endif /* SYS_CPNT_AMTRL3 */
#if (SYS_CPNT_CFM==TRUE)
    SYS_BLD_CFM_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_PPPOE_IA == TRUE)
    SYS_BLD_PPPOE_IA_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
    SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY,
#endif
#if ((SYS_CPNT_DHCPV6SNP == TRUE)||(SYS_CPNT_NDSNP == TRUE))
    SYS_BLD_DHCPSNP_GROUP_IPCMSGQ_KEY,
#endif
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_DCB_GROUP == TRUE)
    SYS_BLD_DCB_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_VXLAN == TRUE)
    SYS_BLD_VXLAN_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T port_link_up_msgqkey_list[] =
{
    0
};

static const UI32_T uport_link_up_msgqkey_list[] =
{
#if ((SYS_CPNT_LACP == TRUE) || (SYS_CPNT_SYNCE == TRUE))
    SYS_BLD_LACP_GROUP_IPCMSGQ_KEY,
#endif
    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_EFM_OAM == TRUE)
    SYS_BLD_OAM_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T uport_fast_link_up_msgqkey_list[] =
{
    0
};

static const UI32_T port_link_down_msgqkey_list[] =
{
    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T uport_link_down_msgqkey_list[] =
{
#if ((SYS_CPNT_LACP == TRUE) || (SYS_CPNT_SYNCE == TRUE))
    SYS_BLD_LACP_GROUP_IPCMSGQ_KEY,
#endif
    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_WEBAUTH == TRUE)
    SYS_BLD_WEB_GROUP_IPCMSGQ_KEY,
#endif /* #if(SYS_CPNT_WEBAUTH == TRUE) */
#if (SYS_CPNT_EFM_OAM == TRUE)
    SYS_BLD_OAM_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T uport_fast_link_down_msgqkey_list[] =
{
    0
};

static const UI32_T port_oper_up_msgqkey_list[] =
{
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_MLAG == TRUE)
    SYS_BLD_MLAG_GROUP_IPCMSGQ_KEY,
#endif
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_CFM==TRUE)
    SYS_BLD_CFM_GROUP_IPCMSGQ_KEY,
#endif
/*  SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY, l2mcat working on port, so use forwarding status is better*/
    0
};

static const UI32_T port_not_oper_up_msgqkey_list[] =
{
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_MLAG == TRUE)
    SYS_BLD_MLAG_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_CFM==TRUE)
    SYS_BLD_CFM_GROUP_IPCMSGQ_KEY,
#endif
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
/*  SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY, l2mcat working on port, so use forwarding status is better*/
    0
};

static const UI32_T trunk_member_port_oper_up_msgqkey_list[] =
{
    0
};

static const UI32_T trunk_member_port_not_oper_up_msgqkey_list[] =
{
    0
};

static const UI32_T trunk_member_active_msgqkey_list[] =
{
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T trunk_member_inactive_msgqkey_list[] =
{
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_AMTRL3 == TRUE)
    SYS_BLD_AMTRL3_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T port_admin_enable_msgqkey_list[] =
{
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T uport_admin_enable_msgqkey_list[] =
{
#if ((SYS_CPNT_LACP == TRUE) || (SYS_CPNT_SYNCE == TRUE))
    SYS_BLD_LACP_GROUP_IPCMSGQ_KEY,
#endif
    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_EFM_OAM == TRUE)
    SYS_BLD_OAM_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T port_admin_disable_msgqkey_list[] =
{
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T port_admin_disable_before_msgqkey_list[] =
{
    0
};

static const UI32_T uport_admin_disable_msgqkey_list[] =
{
#if ((SYS_CPNT_LACP == TRUE) || (SYS_CPNT_SYNCE == TRUE))
    SYS_BLD_LACP_GROUP_IPCMSGQ_KEY,
#endif
    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_EFM_OAM == TRUE)
    SYS_BLD_OAM_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T port_status_changed_passively_msgqkey_list[] =
{
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T port_speed_duplex_msgqkey_list[] =
{
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T uport_speed_duplex_msgqkey_list[] =
{
#if ((SYS_CPNT_LACP == TRUE) || (SYS_CPNT_SYNCE == TRUE))
    SYS_BLD_LACP_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_EFM_OAM == TRUE)
    SYS_BLD_OAM_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T uport_lacp_effective_oper_status_changed_msgqkey_list[] =
{
    0
};

static const UI32_T uport_dot1x_effective_oper_status_changed_msgqkey_list[] =
{
    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T forwarding_uport_add_to_trunk_msgqkey_list[] =
{
    0
};

static const UI32_T forwarding_trunk_member_delete_msgqkey_list[] =
{
    0
};

static const UI32_T forwarding_trunk_member_to_non_forwarding_msgqkey_list[] =
{
    0
};

static const UI32_T if_mau_changed_msgqkey_list[] =
{
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T port_learning_status_changed_msgqkey_list[] =
{
    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
    0
};

/* SWDRV */
static const UI32_T swdrv_port_link_up[] =
{
    SYS_BLD_SWCTRL_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T swdrv_port_link_down[] =
{
    SYS_BLD_SWCTRL_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T swdrv_craft_port_link_up[] =
{
    SYS_BLD_SWCTRL_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T swdrv_craft_port_link_down[] =
{
    SYS_BLD_SWCTRL_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T swdrv_port_type_changed[] =
{
    SYS_BLD_SWCTRL_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T swdrv_port_speed_duplex[] =
{
    SYS_BLD_SWCTRL_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T swdrv_port_flow_ctrl[] =
{
    SYS_BLD_SWCTRL_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T swdrv_hot_swap_insert[] =
{
    0
};
static const UI32_T swdrv_hot_swap_remove[] =
{
    0
};
static const UI32_T swdrv_port_sfp_present[] =
{
    SYS_BLD_SWCTRL_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T swdrv_port_sfp_info[] =
{
    SYS_BLD_SWCTRL_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T swdrv_port_sfp_ddm_info[] =
{
    SYS_BLD_SWCTRL_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T swdrv_port_sfp_ddm_info_measured[] =
{
    SYS_BLD_SWCTRL_GROUP_IPCMSGQ_KEY,
    0
};
#if (SYS_CPNT_CFM == TRUE)
static const UI32_T cfm_defect_notify[] =
{
    0
};
#endif /* #if (SYS_CPNT_CFM == TRUE) */

/* IGMPSNP */
static const UI32_T igmpsnp_status_changed_msgqkey_list[] =
{
    SYS_BLD_L2MUX_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T igmpsnp_router_port_add_msgqkey_list[] =
{
    SYS_BLD_L2MUX_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T igmpsnp_router_port_delete_msgqkey_list[] =
{
    SYS_BLD_L2MUX_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T igmpsnp_group_member_add_msgqkey_list[] =
{
    SYS_BLD_L2MUX_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T igmpsnp_group_member_delete_msgqkey_list[] =
{
    SYS_BLD_L2MUX_GROUP_IPCMSGQ_KEY,
    0
};

/* SYSMGMT */
static const UI32_T powerstatuschanged_msgqkey_list[] =
{
    SYS_BLD_UI_MGR_GROUP_IPCMSGQ_KEY,
    0
};

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
static const UI32_T fanstatuschanged_msgqkey_list[] =
{
    SYS_BLD_UI_MGR_GROUP_IPCMSGQ_KEY,
    0
};
#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
static const UI32_T thermalstatuschanged_msgqkey_list[] =
{
    SYS_BLD_UI_MGR_GROUP_IPCMSGQ_KEY,
    0
};
#endif

static const UI32_T xfpmodulestatuschanged_msgqkey_list[] =
{
    SYS_BLD_UI_MGR_GROUP_IPCMSGQ_KEY,
    0
};

/* SYSDRV */
static const UI32_T sysdrv_alarminputstatuschanged_msgqkey_list[] =
{
    SYS_BLD_SYSMGMT_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T sysdrv_alarmoutputstatuschanged_msgqkey_list[] =
{
    SYS_BLD_SYSMGMT_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_EFM_OAM == TRUE)
    SYS_BLD_OAM_GROUP_IPCMSGQ_KEY,
#endif
    0
};
static const UI32_T sysdrv_powerstatuschanged_msgqkey_list[] =
{
    SYS_BLD_SYSMGMT_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
    SYS_BLD_POE_GROUP_IPCMSGQ_KEY,
#endif
    0
};
#if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
static const UI32_T sysdrv_powertypechanged_msgqkey_list[] =
{
    SYS_BLD_SYSMGMT_GROUP_IPCMSGQ_KEY,
    0
};
#endif

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
static const UI32_T sysdrv_fanstatuschanged_msgqkey_list[] =
{
    SYS_BLD_SYSMGMT_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T sysdrv_fanspeedchanged_msgqkey_list[] =
{
    SYS_BLD_SYSMGMT_GROUP_IPCMSGQ_KEY,
    0
};
#endif
#if (SYS_CPNT_THERMAL_DETECT == TRUE)
static const UI32_T sysdrv_thermalstatuschanged_msgqkey_list[] =
{
    SYS_BLD_SYSMGMT_GROUP_IPCMSGQ_KEY,
    0
};
#endif

static const UI32_T sysdrv_xfpmodulestatuschanged_msgqkey_list[] =
{
    SYS_BLD_SYSMGMT_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T sysdrv_xenpakstatuschanged_msgqkey_list[] =
{
    SYS_BLD_SYSMGMT_GROUP_IPCMSGQ_KEY,
    0
};

/* TRK */
static const UI32_T add_static_trunk_member_msgqkey_list[] =
{
#if ((SYS_CPNT_LACP == TRUE) || (SYS_CPNT_SYNCE == TRUE))
    SYS_BLD_LACP_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T del_static_trunk_member_msgqkey_list[] =
{
#if ((SYS_CPNT_LACP == TRUE) || (SYS_CPNT_SYNCE == TRUE))
    SYS_BLD_LACP_GROUP_IPCMSGQ_KEY,
#endif
    0
};

/* LAN */
static const UI32_T lan_receive_l2mux_packet_msgqkey_list[] =
{
    SYS_BLD_L2MUX_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T lan_receive_lacp_packet_msgqkey_list[] =
{
#if ((SYS_CPNT_LACP == TRUE) || (SYS_CPNT_SYNCE == TRUE))
    SYS_BLD_LACP_GROUP_IPCMSGQ_KEY,
#endif
    0
};
static const UI32_T lan_receive_oam_packet_msgqkey_list[] =
{
#if (SYS_CPNT_EFM_OAM == TRUE)
    SYS_BLD_OAM_GROUP_IPCMSGQ_KEY,
#endif
    0
};
static const UI32_T lan_receive_oam_lbk_packet_msgqkey_list[] =
{
#if (SYS_CPNT_EFM_OAM_REMOTE_LB_ACTIVELY == TRUE)
    SYS_BLD_OAM_GROUP_LBK_IPCMSGQ_KEY,
#endif
    0
};
static const UI32_T lan_receive_loopback_packet_msgqkey_list[] =
{
#if (SYS_CPNT_INTERNAL_LOOPBACK_TEST == TRUE)
    SYS_BLD_SWCTRL_GROUP_IPCMSGQ_KEY,
#endif
    0
};
static const UI32_T lan_receive_dot1x_packet_msgqkey_list[] =
{
    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T lan_receive_sflow_packet_msgqkey_list[] =
{
#if (SYS_CPNT_SFLOW == TRUE)
    SYS_BLD_SFLOW_GROUP_IPCMSGQ_KEY,
#endif
    0
};
static const UI32_T lan_receive_lbd_packet_msgqkey_list[] =
{
    0
};

static const UI32_T lan_receive_udld_packet_msgqkey_list[]=
{
    0
};

static const UI32_T lan_receive_authenticate_packet_msgqkey_list[]=
{
    SYS_BLD_SYS_CALLBACK_GROUP_IPCMSGQ_KEY,
    0
};


static const UI32_T lan_receive_esmc_packet_msgqkey_list[] =
{
#if ((SYS_CPNT_LACP == TRUE) || (SYS_CPNT_SYNCE == TRUE))
    SYS_BLD_LACP_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T lan_receive_mlag_packet_msgqkey_list[] =
{
#if (SYS_CPNT_MLAG == TRUE)
    SYS_BLD_MLAG_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T lan_authentication_dispatch_msgqkey_list[] =
{
    SYS_BLD_DRIVER_GROUP_DISPATCHPKT_IPCMSGQ_KEY,
    0
};

/* ISC */
static const UI32_T isc_receive_stktplg_packet_msgqkey_list[] =
{
    SYS_BLD_CSC_STKTPLG_TASK_MSGK_KEY,
    0
};

/* IML */
static const UI32_T l2mux_receive_ip_packet_msgqkey_list[] =
{
    SYS_BLD_L2MUX_GROUP_IPCMSGQ_KEY,
    0
};

/* L2MUX */
static const UI32_T l2mux_receive_sta_packet_msgqkey_list[] =
{
    SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T l2mux_receive_gvrp_packet_msgqkey_list[] =
{
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T l2mux_receive_lldp_packet_msgqkey_list[] =
{
    SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T l2mux_receive_igmpsnp_packet_msgqkey_list[] =
{
#if(SYS_CPNT_L2MCAST == TRUE)
    SYS_BLD_L2MCAST_GROUP_DATA_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T l2mux_receive_mldsnp_packet_msgqkey_list[]=
{
#if(SYS_CPNT_L2MCAST == TRUE)
    SYS_BLD_L2MCAST_GROUP_DATA_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T l2mux_receive_cfm_packet_msgqkey_list[]=
{
#if (SYS_CPNT_CFM==TRUE)
    SYS_BLD_CFM_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T l2mux_receive_elps_packet_msgqkey_list[]=
{
    0
};

static const UI32_T l2mux_receive_pppoed_packet_msgqkey_list[]=
{
#if (SYS_CPNT_PPPOE_IA == TRUE)
    SYS_BLD_PPPOE_IA_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T l2mux_receive_raps_packet_msgqkey_list[]=
{
    0
};

static const UI32_T l2mux_receive_erps_health_packet_msgqkey_list[]=
{
    0
};

static const UI32_T l2mux_receive_cluster_packet_msgqkey_list[]=
{
    SYS_BLD_CLUSTER_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T l2mux_receive_ptp_packet_msgqkey_list[] =
{
    0
};

static const UI32_T l2mux_rx_snoop_dhcp_packet_msgqkey_list[] =
{
    SYS_BLD_SYS_CALLBACK_GROUP_IPCMSGQ_KEY,
    0
};

/* IML */
static const UI32_T iml_receive_bootp_packet_msgqkey_list[] =
{
    SYS_BLD_IP_SERVICE_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T iml_receive_udp_helper_packet_msgqkey_list[] =
{
    SYS_BLD_UDPHELPER_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T iml_receive_arp_packet_msgqkey_list[] =
{
    SYS_BLD_DAI_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T iml_receive_hsrp_packet_msgqkey_list[] =
{
    //SYS_BLD_IP_SERVICE_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T iml_receive_vrrp_packet_msgqkey_list[] =
{
#if (SYS_CPNT_VRRP == TRUE)
    SYS_BLD_VRRP_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T iml_receive_dhcpsnp_packet_msgqkey_list[] =
{
#if (SYS_CPNT_DHCPSNP == TRUE)
    SYS_BLD_DHCPSNP_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T iml_receive_raguard_packet_msgqkey_list[] =
{
#if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE)
    SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T iml_tx_snoop_dhcp_packet_msgqkey_list[] =
{
#if (SYS_CPNT_DHCPSNP == TRUE)
    SYS_BLD_DHCPSNP_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T iml_receive_dhcpv6snp_packet_msgqkey_list[]=
{
#if (SYS_CPNT_DHCPV6SNP == TRUE)
    SYS_BLD_DHCPSNP_GROUP_IPCMSGQ_KEY,
#endif
    0
};

/* STKTPLG */
static const UI32_T stack_state_msgqkey_list[] =
{
    SYS_BLD_STKCTRL_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T module_state_changed_msgqkey_list[] =
{
    SYS_BLD_STKCTRL_GROUP_IPCMSGQ_KEY,
    0
};
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
static const UI32_T unit_hot_swap_msgqkey_list[] =
{
    SYS_BLD_STKCTRL_GROUP_IPCMSGQ_KEY,
    0
};
#endif

/* STKCTRL */
static const UI32_T saving_config_status_msgqkey_list[] =
{
    SYS_BLD_CLI_GROUP_IPCMSGQ_KEY,
    0
};

/* NETCFG */
/* Triggered by IPCFG
 */
static const UI32_T rif_created_msgqkey_list[] =
{
    SYS_BLD_IP_SERVICE_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T rif_active_msgqkey_list[] =
{
    SYS_BLD_UTILITY_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_VRRP == TRUE)
    SYS_BLD_VRRP_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)
    SYS_BLD_XFER_GROUP_IPCMSGQ_KEY,
#endif /* #if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE) */
#if (SYS_CPNT_DHCPV6_RELAY == TRUE)
    SYS_BLD_IP_SERVICE_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_VXLAN == TRUE)
    SYS_BLD_VXLAN_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_OVSVTEP == TRUE)
    SYS_BLD_OVSVTEP_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_NTP == TRUE)
    SYS_BLD_APP_PROTOCOL_GROUP_IPCMSGQ_KEY,
#endif /* #if (SYS_CPNT_NTP == TRUE) */
    0
};

static const UI32_T rif_down_msgqkey_list[] =
{
    SYS_BLD_UTILITY_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_VRRP == TRUE)
    SYS_BLD_VRRP_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_VXLAN == TRUE)
    SYS_BLD_VXLAN_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_OVSVTEP == TRUE)
    SYS_BLD_OVSVTEP_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T rif_destroyed_msgqkey_list[] =
{
    SYS_BLD_IP_SERVICE_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_TELNET == TRUE)
    SYS_BLD_TELNET_GROUP_IPCMSGQ_KEY,
#endif
    SYS_BLD_WEB_GROUP_IPCMSGQ_KEY,
    SYS_BLD_CLI_GROUP_IPCMSGQ_KEY,
    0
};

/*Donny.li modify for VRRP */
static const UI32_T netcfg_l3if_destroy_msgqkey_list[] =
{
    SYS_BLD_IP_SERVICE_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_VRRP == TRUE)
    SYS_BLD_VRRP_GROUP_IPCMSGQ_KEY,
#endif
    0
};
/*Donny.li modify for VRRP end*/

#if (SYS_CPNT_DHCPV6== TRUE)
static const UI32_T netcfg_ipv6_addrautoconfig_msgqkey_list[] =
{
    SYS_BLD_IP_SERVICE_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T netcfg_l3if_create_msgqkey_list[] =
{
    SYS_BLD_IP_SERVICE_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T netcfg_l3if_oper_status_up_msgqkey_list[] =
{
    SYS_BLD_IP_SERVICE_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T netcfg_l3if_oper_status_down_msgqkey_list[] =
{
    SYS_BLD_IP_SERVICE_GROUP_IPCMSGQ_KEY,
    0
};
#endif

/* CLI */
/* for cli provision complete notify CSC
 */
static const UI32_T provision_complete_msgqkey_list[] =
{
    SYS_BLD_STKCTRL_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T module_provision_complete_msgqkey_list[] =
{
    SYS_BLD_STKCTRL_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T enter_transition_mode_msgqkey_list[] =
{
    SYS_BLD_STKCTRL_GROUP_IPCMSGQ_KEY,
    0
};

/* LLDP */
static const UI32_T telephone_detect_msgqkey_list[] =
{
    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
    0
};

#ifdef SYS_CPNT_POE_PSE_DOT3AT
static const UI32_T dot3at_info_received_msgqkey_list[] =
{
    SYS_BLD_POE_GROUP_IPCMSGQ_KEY,
    0
};
#endif

#if (SYS_CPNT_CN == TRUE)
static const UI32_T cn_remote_change_msgqkey_list[] =
{
    SYS_BLD_DCB_GROUP_IPCMSGQ_KEY,
};
#endif

/*maggie liu for RADIUS authentication ansync*/
static const UI32_T radius_authen_result_msgqkey_list[] =
{
    SYS_BLD_SYSMGMT_GROUP_IPCMSGQ_KEY,
    0
};


/* AMTRL3 */
#if (SYS_CPNT_AMTRL3 == TRUE)
static const UI32_T nexthop_status_change_msgqkey_list[] =
{
    /* pbr... */
    0
};
#endif

#if(SYS_CPNT_DHCP == TRUE)
static const UI32_T dhcp_restart3_msgqkey_list[] =
{
    SYS_BLD_IP_SERVICE_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T dhcp_setifrole_msgqkey_list[] =
{
    SYS_BLD_IP_SERVICE_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T dhcp_setifstatus_msgqkey_list[] =
{
    SYS_BLD_IP_SERVICE_GROUP_IPCMSGQ_KEY,
    0
};
#endif

#if(SYS_CPNT_CLI_DYNAMIC_PROVISION_VIA_DHCP == TRUE)
static const UI32_T dhcp_rxoptionconfig_msgqkey_list[] =
{
    SYS_BLD_CLI_GROUP_IPCMSGQ_KEY,
    0
};
#endif

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
static const UI32_T sysmgmt_announce_remain_date_msgqkey_list[] =
{
    SYS_BLD_CLI_GROUP_IPCMSGQ_KEY,
    0
};
#endif
#if (SYS_CPNT_IP_TUNNEL == TRUE)
static const UI32_T netcfg_announce_ipv6_msgqkey_list[] =
{
    /* SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY, */ /* Currently unused */
    0
};

static const UI32_T amtrl3_tunnel_net_route_hit_bit_change_msgqkey_list[] =
{
    SYS_BLD_AMTRL3_GROUP_IPCMSGQ_KEY,
    0
};
#endif /*SYS_CPNT_IP_TUNNEL*/

#if (SYS_CPNT_POE == TRUE)
static const UI32_T poedrv_port_detection_status_change_msgqkey_list[] =
{
    SYS_BLD_POE_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T poedrv_port_status_change_msgqkey_list[] =
{
    SYS_BLD_POE_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T poedrv_is_main_power_reach_maximum_msgqkey_list[] =
{
    SYS_BLD_POE_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T poedrv_port_overload_status_change_msgqkey_list[] =
{
    SYS_BLD_POE_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T poedrv_port_power_consumption_change_msgqkey_list[] =
{
    SYS_BLD_POE_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T poedrv_port_power_classification_change_msgqkey_list[] =
{
    SYS_BLD_POE_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T poedrv_main_pse_consumption_change_msgqkey_list[] =
{
    SYS_BLD_POE_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T poedrv_pse_oper_status_change_msgqkey_list[] =
{
    SYS_BLD_POE_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T poedrv_power_denied_occur_frenquently_msgqkey_list[] =
{
    SYS_BLD_POE_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T poedrv_port_failure_status_change_msgqkey_list[] =
{
    SYS_BLD_POE_GROUP_IPCMSGQ_KEY,
    0
};
#endif

#if (SYS_CPNT_CLUSTER == TRUE)
static const UI32_T cluster_changerole_msgqkey_list[] =
{
    SYS_BLD_SNMP_GROUP_IPCMSGQ_KEY,
    0
};
#endif

#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
static const UI32_T acl_deleted_msgqkey_list[] =
{
    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
    0
};

static const UI32_T policy_map_deleted_msgqkey_list[] =
{
    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY,
    0
};
#endif /* #if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE) */

#if (SYS_CPNT_COS == TRUE)
static const UI32_T cos_port_config_changed_msgqkey_list[] =
{
#if (SYS_CPNT_DCB_GROUP == TRUE)
    SYS_BLD_DCB_GROUP_IPCMSGQ_KEY,
#endif
    0
};
#endif /* #if (SYS_CPNT_COS == TRUE) */

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
static const UI32_T mgmt_ip_flt_changed_msgqkey_list[] =
{
#if (SYS_CPNT_TELNET == TRUE)
    SYS_BLD_TELNET_GROUP_IPCMSGQ_KEY,
#endif
    SYS_BLD_WEB_GROUP_IPCMSGQ_KEY,
    0
};
#endif /* #if (SYS_CPNT_MGMT_IP_FLT == TRUE) */

static const UI32_T nsm_route_change_msgqkey_list[] =
{
    SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY,
#if (SYS_CPNT_VXLAN == TRUE)
    SYS_BLD_VXLAN_GROUP_IPCMSGQ_KEY,
#endif
#if (SYS_CPNT_OVSVTEP == TRUE)
    SYS_BLD_OVSVTEP_GROUP_IPCMSGQ_KEY,
#endif
    0
};

static const UI32_T set_port_status_msgqkey_list[] =
{
    SYS_BLD_CMGR_GROUP_IPCMSGQ_KEY,
};

#if (SYS_CPNT_DCBX == TRUE)
static const UI32_T ets_received_msgqkey_list[] =
{
    SYS_BLD_DCB_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T pfc_received_msgqkey_list[] =
{
    SYS_BLD_DCB_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T ets_cfg_changed_msgqkey_list[] =
{
    SYS_BLD_DCB_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T pfc_cfg_changed_msgqkey_list[] =
{
    SYS_BLD_DCB_GROUP_IPCMSGQ_KEY,
    0
};
#endif  /* #if (SYS_CPNT_DCBX == TRUE) */

#if (SYS_CPNT_IGMPAUTH == TRUE)
static const UI32_T igmp_msgqkey_list[] =
{
    SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY,
    0
};
#endif

#if (SYS_CPNT_PBR == TRUE)
static const UI32_T host_route_changed_msgqkey_list[] =
{
    SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T acl_changed_msgqkey_list[] =
{
    SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY,
    0
};
static const UI32_T routemap_changed_msgqkey_list[] =
{
    SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY,
    0
};
#endif

/* SYS_CALLBACK_GROUP */
static const UI32_T sys_callback_rx_dhcp_packet_msgqkey_list[] =
{
    SYS_BLD_L2MUX_GROUP_IPCMSGQ_KEY,
    0
};
static BOOL_T SYS_CALLBACK_MGR_GetExtHdrLen(UI8_T *payload_p, UI8_T ext_hdr_type, UI32_T *ext_hdr_len_p);
static BOOL_T SYS_CALLBACK_MGR_GetTotalExtHdrLen(UI8_T *payload_p, UI32_T *total_ext_hdr_len_p, UI32_T *next_hdr_type_p);
static void SYS_CALLBACK_MGR_Backdoor_DebugIpc(UI32_T src_csc_id, UI32_T dest_msgq_key, SYSFUN_Msg_T *sysfun_msg_p, UI32_T sysfun_ret);
static void SYS_CALLBACK_MGR_Backdoor_DebugSyscbMsg(SYS_CALLBACK_MGR_Msg_T *syscb_msg_p);


/* EXPORTED SUBPROGRAM BODIES
 */

/************************
 *  Stacking mode APIs  *
 ************************
 */
/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_CALLBACK_MGR_EnterMasterMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Let SYS_CALLBACK enter master mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SYS_CALLBACK_MGR_EnterMasterMode(void)
{
    SYS_CALLBACK_OM_EnterMasterMode();
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_CALLBACK_MGR_EnterSlaveMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Let SYS_CALLBACK enter slave mode.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SYS_CALLBACK_MGR_EnterSlaveMode(void)
{
    SYS_CALLBACK_OM_EnterSlaveMode();
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_CALLBACK_MGR_EnterTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Let SYS_CALLBACK enter transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SYS_CALLBACK_MGR_EnterTransitionMode(void)
{
    SYS_CALLBACK_OM_EnterTransitionMode();
    SYS_CALLBACK_OM_ResetAllFailEntries();
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_CALLBACK_MGR_SetTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Set SYS_CALLBACK to temporary transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SYS_CALLBACK_MGR_SetTransitionMode(void)
{
    SYS_CALLBACK_OM_SetTransitionMode();
}

/*********************************
 *  Notify callback events APIs  *
 *********************************
 */

/*********************************
 *              AMTR             *
 *********************************
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_MGR_IntrusionMacCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When detecting intrusion mac, AMTR will notify other CSC groups by this
 *      function.
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      src_lport        -- which lport
 *      vid              -- which vlan id
 *      src_mac          -- source mac address
 *      dst_mac          -- destination mac address
 *      ether_type       -- ether type
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_IntrusionMacCallback(UI32_T src_csc_id, UI32_T src_lport, UI16_T vid, UI8_T *src_mac, UI8_T *dst_mac, UI16_T ether_type)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_IntrusionMac_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_IntrusionMac_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_IntrusionMac_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_IntrusionMac_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_INTRUSION_MAC;
    cbdata_msg_p->src_lport = src_lport;
    cbdata_msg_p->vid       = vid;
    cbdata_msg_p->ether_type = ether_type;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_PortMoveCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When port move, AMTR will notify other CSC groups by this function.
 *
 * INPUT:
 *      src_csc_id      -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      num_of_entries  -- number of port move entries
 *      buf             -- port move entries buffer
 *      buf_size        -- port move entries buffer size
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_PortMoveCallback(UI32_T src_csc_id, UI32_T num_of_entries, UI8_T *buf, UI32_T buf_size)
{
    SYSFUN_Msg_T                        *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T              *syscb_msg_p;
    SYS_CALLBACK_MGR_PortMove_CBData_T  *cbdata_msg_p;
    UI8_T        *ipcmsg_buf;
    BOOL_T       return_value;
    UI32_T       total_msg_size;

    total_msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_PortMove_CBData_T))+buf_size;
    ipcmsg_buf = L_MM_Malloc(SYSFUN_SIZE_OF_MSG(total_msg_size),L_MM_USER_ID2(SYS_MODULE_SYS_CALLBACK, EXT_TRACE_ID_PORTMOVECALLBACK));
    if (ipcmsg_buf == NULL)
    {
        return FALSE;
    }
    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = total_msg_size;
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_PortMove_CBData_T*)syscb_msg_p->callback_data;
    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_MOVE;
    cbdata_msg_p->number_of_entry = num_of_entries;
    memcpy(cbdata_msg_p->buf, buf, buf_size);
    return_value = SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
    L_MM_Free(ipcmsg_buf);
    return return_value;
}


/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_SecurityPortMoveCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When port move, AMTR will notify other CSC groups by this function.
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      ifindex          -- port whcih the mac is learnt now
 *      vid              -- which vlan id
 *      mac              -- mac address
 *      original_ifindex -- original port which the mac was learnt before
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_SecurityPortMoveCallback(UI32_T src_csc_id, UI32_T ifindex, UI32_T vid, UI8_T  *mac, UI32_T original_ifindex)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_SecurityPortMove_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SecurityPortMove_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SecurityPortMove_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_SecurityPortMove_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SECURITY_PORT_MOVE;
    cbdata_msg_p->ifindex = ifindex;
    cbdata_msg_p->vid     = vid;
    memcpy(cbdata_msg_p->mac, mac, sizeof(cbdata_msg_p->mac));
    cbdata_msg_p->original_ifindex = original_ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_AutoLearnCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : When learning arrive learn_with_count, AMTR will notify other CSCs
 *           by this function.
 *
 * INPUT   : src_csc_id     -- The csc_id who triggers this event (SYS_MODULE_AMTR)
 *           ifindex        --
 *           portsec_status --
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_AutoLearnCallback(UI32_T src_csc_id, UI32_T ifindex,
                                          UI32_T portsec_status)
{
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_AutoLearn_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AutoLearn_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AutoLearn_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_AutoLearn_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_AUTO_LEARN;
    cbdata_msg_p->ifindex = ifindex;
    cbdata_msg_p->portsec_status = portsec_status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_MACTableDeleteByPortCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : The registered callback is invoked when a delete by port command
 *           is issued and AMTRDRV has cleared the address table of lower layer.
 *
 * INPUT   : src_csc_id -- The csc_id who triggers this event (SYS_MODULE_AMTR)
 *           ifindex    --
 *           reason     --
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_MACTableDeleteByPortCallback(UI32_T src_csc_id,
                                                     UI32_T ifindex,
                                                     UI32_T reason)
{
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_MACTableDeleteByPort_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_MACTableDeleteByPort_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_MACTableDeleteByPort_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_MACTableDeleteByPort_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_TABLE_DELETE_BY_PORT;
    cbdata_msg_p->ifindex = ifindex;
    cbdata_msg_p->reason = reason;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_MACTableDeleteByVidCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : The registered callback is invoked when a delete by vid command is
 *           issued and AMTRDRV has cleared the address table of lower layer.
 *
 * INPUT   : src_csc_id -- The csc_id who triggers this event (SYS_MODULE_AMTR)
 *           vid        -- which vlan id
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_MACTableDeleteByVidCallback(UI32_T src_csc_id, UI32_T vid)
{
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_MACTableDeleteByVid_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_MACTableDeleteByVid_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_MACTableDeleteByVid_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_MACTableDeleteByVid_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_TABLE_DELETE_BY_VID;
    cbdata_msg_p->vid = vid;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_MACTableDeleteByVIDnPortCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : The registered callback is invoked when a delete by vid+port command is issued
 *           and AMTRDRV has cleared the address table of lower layer.
 *
 * INPUT   : src_csc_id -- The csc_id who triggers this event (SYS_MODULE_AMTR)
 *           vid        -- which vlan id
 *           ifindex    --
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_MACTableDeleteByVIDnPortCallback(UI32_T src_csc_id,
                                                         UI32_T vid,
                                                         UI32_T ifindex)
{
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_MACTableDeleteByVIDnPort_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_MACTableDeleteByVIDnPort_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_MACTableDeleteByVIDnPort_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_MACTableDeleteByVIDnPort_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_TABLE_DELETE_BY_VID_AND_PORT;
    cbdata_msg_p->vid = vid;
    cbdata_msg_p->ifindex = ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_MacTableDeleteByLifeTimeCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : The registered callback is invoked when a delete by life time command is issued
 *           and AMTRDRV has cleared the address table of lower layer.
 *
 * INPUT   : src_csc_id -- The csc_id who triggers this event (SYS_MODULE_AMTR)
 *           life_time  -- life time
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_MacTableDeleteByLifeTimeCallback(UI32_T src_csc_id,
                                                         UI32_T life_time)
{
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_MacTableDeleteByLifeTime_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_MacTableDeleteByLifeTime_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_MacTableDeleteByLifeTime_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_MacTableDeleteByLifeTime_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_TABLE_DELETE_BY_LIFE_TIME;
    cbdata_msg_p->life_time = life_time;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*********************************
 *              VLAN             *
 *********************************
 */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_VlanCreateCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when a vlan is created.
 *
 * INPUT   : src_csc_id  -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           vid_ifindex -- specify which vlan has just been created
 *           vlan_status -- VAL_dot1qVlanStatus_other \
 *                          VAL_dot1qVlanStatus_permanent \
 *                          VAL_dot1qVlanStatus_dynamicGvrp
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_VlanCreateCallback(UI32_T src_csc_id,
                                           UI32_T vid_ifindex,
                                           UI32_T vlan_status)
{
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    if (SYS_CALLBACK_OM_SetChange_2(SYS_CALLBACK_OM_KIND_VLAN, vid_ifindex -
            SYS_ADPT_VLAN_1_IF_INDEX_NUMBER + 1, SYS_CALLBACK_OM_ACTION_TO_ON)
            == TRUE)
    {
        return SYS_CALLBACK_MGR_NotifyCmgrGroup();
    }
    else
    {
        return FALSE;
    }
#else
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_VlanCreate_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanCreate_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanCreate_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_VlanCreate_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_CREATE;
    cbdata_msg_p->vid_ifindex = vid_ifindex;
    cbdata_msg_p->vlan_status = vlan_status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_VlanCreateForGVRPCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when a vlan is created not by GVRP.
 *
 * INPUT   : src_csc_id  -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           vid_ifindex -- specify which vlan has just been created
 *           vlan_status -- VAL_dot1qVlanStatus_other \
 *                          VAL_dot1qVlanStatus_permanent
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_VlanCreateForGVRPCallback(UI32_T src_csc_id,
                                                  UI32_T vid_ifindex,
                                                  UI32_T vlan_status)
{
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    if (SYS_CALLBACK_OM_SetChange_2(SYS_CALLBACK_OM_KIND_GVRP_VLAN, vid_ifindex
            - SYS_ADPT_VLAN_1_IF_INDEX_NUMBER + 1,
            SYS_CALLBACK_OM_ACTION_TO_ON) == TRUE)
    {
        return SYS_CALLBACK_MGR_NotifyCmgrGroup();
    }
    else
    {
        return FALSE;
    }
#else
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_VlanCreate_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanCreate_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanCreate_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_VlanCreate_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_CREATE_FOR_GVRP;
    cbdata_msg_p->vid_ifindex = vid_ifindex;
    cbdata_msg_p->vlan_status = vlan_status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_VlanDestroyCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when a vlan is deleted.
 *
 * INPUT   : src_csc_id  -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           vid_ifindex -- specify which vlan has just been deleted
 *           vlan_status -- VAL_dot1qVlanStatus_other \
 *                          VAL_dot1qVlanStatus_permanent \
 *                          VAL_dot1qVlanStatus_dynamicGvrp
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_VlanDestroyCallback(UI32_T src_csc_id,
                                            UI32_T vid_ifindex,
                                            UI32_T vlan_status)
{
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    if (SYS_CALLBACK_OM_SetChange_2(SYS_CALLBACK_OM_KIND_VLAN, vid_ifindex -
            SYS_ADPT_VLAN_1_IF_INDEX_NUMBER + 1,
            SYS_CALLBACK_OM_ACTION_TO_OFF) == TRUE)
    {
        return SYS_CALLBACK_MGR_NotifyCmgrGroup();
    }
    else
    {
        return FALSE;
    }
#else
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_VlanDestroy_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanDestroy_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanDestroy_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_VlanDestroy_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_DESTROY;
    cbdata_msg_p->vid_ifindex = vid_ifindex;
    cbdata_msg_p->vlan_status = vlan_status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_VlanDestroyForGVRPCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when a vlan is deleted not by GVRP.
 *
 * INPUT   : src_csc_id  -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           vid_ifindex -- specify which vlan has just been deleted
 *           vlan_status -- VAL_dot1qVlanStatus_other \
 *                          VAL_dot1qVlanStatus_permanent
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_VlanDestroyForGVRPCallback(UI32_T src_csc_id,
                                                   UI32_T vid_ifindex,
                                                   UI32_T vlan_status)
{
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    if (SYS_CALLBACK_OM_SetChange_2(SYS_CALLBACK_OM_KIND_GVRP_VLAN, vid_ifindex
            - SYS_ADPT_VLAN_1_IF_INDEX_NUMBER + 1,
            SYS_CALLBACK_OM_ACTION_TO_OFF) == TRUE)
    {
        return SYS_CALLBACK_MGR_NotifyCmgrGroup();
    }
    else
    {
        return FALSE;
    }
#else
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_VlanDestroy_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanDestroy_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanDestroy_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_VlanDestroy_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_DESTROY_FOR_GVRP;
    cbdata_msg_p->vid_ifindex = vid_ifindex;
    cbdata_msg_p->vlan_status = vlan_status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_L3VlanDestroyCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when a vlan is deleted.
 *
 * INPUT   : src_csc_id  -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           vid_ifindex -- specify which vlan has just been deleted
 *           vlan_status -- VAL_dot1qVlanStatus_other \
 *                          VAL_dot1qVlanStatus_permanent \
 *                          VAL_dot1qVlanStatus_dynamicGvrp
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_L3VlanDestroyCallback(UI32_T src_csc_id,
                                            UI32_T vid_ifindex,
                                            UI32_T vlan_status)
{
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    if (SYS_CALLBACK_OM_SetChange_1(SYS_CALLBACK_OM_KIND_L3_VLAN, vid_ifindex -
            SYS_ADPT_VLAN_1_IF_INDEX_NUMBER + 1) == TRUE)
    {
        return SYS_CALLBACK_MGR_NotifyCmgrGroup();
    }
    else
    {
        return FALSE;
    }
#else
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_VlanDestroy_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanDestroy_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanDestroy_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_VlanDestroy_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_L3VLAN_DESTROY;
    cbdata_msg_p->vid_ifindex = vid_ifindex;
    cbdata_msg_p->vlan_status = vlan_status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_VlanMemberAddCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when a lport is added to a vlan's member
 *           set.
 *
 * INPUT   : src_csc_id    -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           vid_ifindex   -- specify which vlan member set to join
 *           lport_ifindex -- sepcify which lport to join to the member set
 *           vlan_status   -- VAL_dot1qVlanStatus_other
 *                            VAL_dot1qVlanStatus_permanent
 *                            VAL_dot1qVlanStatus_dynamicGvrp
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_VlanMemberAddCallback(UI32_T src_csc_id,
                                              UI32_T vid_ifindex,
                                              UI32_T lport_ifindex,
                                              UI32_T vlan_status)
{
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    if (SYS_CALLBACK_OM_SetChange_3(SYS_CALLBACK_OM_KIND_VLAN_MEMBER,
            vid_ifindex - SYS_ADPT_VLAN_1_IF_INDEX_NUMBER + 1, lport_ifindex,
            SYS_CALLBACK_OM_ACTION_TO_ON) == FALSE)
    {
        return FALSE;
    }
    else if (SYS_CALLBACK_OM_SetChange_1(SYS_CALLBACK_OM_KIND_PORT_VLAN,
                lport_ifindex) == FALSE)
    {
        return FALSE;
    }
    else
    {
        return SYS_CALLBACK_MGR_NotifyCmgrGroup();
    }
#else
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_VlanMemberAdd_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanMemberAdd_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanMemberAdd_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_VlanMemberAdd_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_ADD;
    cbdata_msg_p->vid_ifindex = vid_ifindex;
    cbdata_msg_p->lport_ifindex = lport_ifindex;
    cbdata_msg_p->vlan_status = vlan_status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_VlanMemberAddForGVRPCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when a lport is added to a vlan's member
 *           set not by GVRP.
 *
 * INPUT   : src_csc_id    -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           vid_ifindex   -- specify which vlan member set to join
 *           lport_ifindex -- sepcify which lport to join to the member set
 *           vlan_status   -- VAL_dot1qVlanStatus_other
 *                            VAL_dot1qVlanStatus_permanent
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_VlanMemberAddForGVRPCallback(UI32_T src_csc_id,
                                                     UI32_T vid_ifindex,
                                                     UI32_T lport_ifindex,
                                                     UI32_T vlan_status)
{
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    if (SYS_CALLBACK_OM_SetChange_3(SYS_CALLBACK_OM_KIND_GVRP_VLAN_MEMBER,
            vid_ifindex - SYS_ADPT_VLAN_1_IF_INDEX_NUMBER + 1, lport_ifindex,
            SYS_CALLBACK_OM_ACTION_TO_ON) == TRUE)
    {
        return SYS_CALLBACK_MGR_NotifyCmgrGroup();
    }
    else
    {
        return FALSE;
    }
#else
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_VlanMemberAdd_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanMemberAdd_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanMemberAdd_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_VlanMemberAdd_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_ADD_FOR_GVRP;
    cbdata_msg_p->vid_ifindex = vid_ifindex;
    cbdata_msg_p->lport_ifindex = lport_ifindex;
    cbdata_msg_p->vlan_status = vlan_status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_VlanMemberDeleteCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when a port is removed from vlan's member
 *           set.
 *
 * INPUT   : src_csc_id    -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           vid_ifindex   -- specify which vlan's member set to disjoin from
 *           lport_ifindex -- sepcify which lport to disjoin from the member set
 *           vlan_status   -- VAL_dot1qVlanStatus_other
 *                            VAL_dot1qVlanStatus_permanent
 *                            VAL_dot1qVlanStatus_dynamicGvrp
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_VlanMemberDeleteCallback(UI32_T src_csc_id,
                                                 UI32_T vid_ifindex,
                                                 UI32_T lport_ifindex,
                                                 UI32_T vlan_status)
{
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    if (SYS_CALLBACK_OM_SetChange_3(SYS_CALLBACK_OM_KIND_VLAN_MEMBER,
            vid_ifindex - SYS_ADPT_VLAN_1_IF_INDEX_NUMBER + 1, lport_ifindex,
            SYS_CALLBACK_OM_ACTION_TO_OFF) == FALSE)
    {
        return FALSE;
    }
    else if (SYS_CALLBACK_OM_SetChange_1(SYS_CALLBACK_OM_KIND_PORT_VLAN,
                lport_ifindex) == FALSE)
    {
        return FALSE;
    }
    else
    {
        return SYS_CALLBACK_MGR_NotifyCmgrGroup();
    }
#else
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_VlanMemberDelete_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanMemberDelete_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanMemberDelete_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_VlanMemberDelete_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE;
    cbdata_msg_p->vid_ifindex = vid_ifindex;
    cbdata_msg_p->lport_ifindex = lport_ifindex;
    cbdata_msg_p->vlan_status = vlan_status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_VlanMemberDeleteForGVRPCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when a port is removed from vlan's member
 *           set not by GVRP.
 *
 * INPUT   : src_csc_id    -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           vid_ifindex   -- specify which vlan's member set to disjoin from
 *           lport_ifindex -- sepcify which lport to disjoin from the member set
 *           vlan_status   -- VAL_dot1qVlanStatus_other
 *                            VAL_dot1qVlanStatus_permanent
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_VlanMemberDeleteForGVRPCallback(UI32_T src_csc_id,
                                                        UI32_T vid_ifindex,
                                                        UI32_T lport_ifindex,
                                                        UI32_T vlan_status)
{
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    if (SYS_CALLBACK_OM_SetChange_3(SYS_CALLBACK_OM_KIND_GVRP_VLAN_MEMBER,
            vid_ifindex - SYS_ADPT_VLAN_1_IF_INDEX_NUMBER + 1, lport_ifindex,
            SYS_CALLBACK_OM_ACTION_TO_OFF) == TRUE)
    {
        return SYS_CALLBACK_MGR_NotifyCmgrGroup();
    }
    else
    {
        return FALSE;
    }
#else
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_VlanMemberDelete_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanMemberDelete_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanMemberDelete_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_VlanMemberDelete_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE_FOR_GVRP;
    cbdata_msg_p->vid_ifindex = vid_ifindex;
    cbdata_msg_p->lport_ifindex = lport_ifindex;
    cbdata_msg_p->vlan_status = vlan_status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_PvidChangeCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when the pvid of a port changes.
 *
 * INPUT   : src_csc_id    -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           lport_ifindex -- the specific port this modification is of
 *           old_pvid      -- previous pvid before modification
 *           new_pvid      -- new and current pvid after modification
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_PvidChangeCallback(UI32_T src_csc_id,
                                           UI32_T lport_ifindex,
                                           UI32_T old_pvid,
                                           UI32_T new_pvid)
{
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    if (SYS_CALLBACK_OM_SetChange_3(SYS_CALLBACK_OM_KIND_PVID, lport_ifindex,
            old_pvid, new_pvid) == TRUE)
    {
        return SYS_CALLBACK_MGR_NotifyCmgrGroup();
    }
    else
    {
        return FALSE;
    }
#else
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_PvidChange_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_PvidChange_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_PvidChange_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_PvidChange_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_PVID_CHANGE;
    cbdata_msg_p->lport_ifindex = lport_ifindex;
    cbdata_msg_p->old_pvid = old_pvid;
    cbdata_msg_p->new_pvid = new_pvid;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_L3IfOperStatusChangedCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when vlan's operation status has changed.
 *
 * INPUT   : src_csc_id   -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           vid_ifindex  -- specify which vlan's status changed
 *           oper_status  -- specify the new status of vlan
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_L3IfOperStatusChangedCallback(UI32_T src_csc_id,
                                                      UI32_T vid_ifindex,
                                                      UI32_T oper_status)
{
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    if (SYS_CALLBACK_OM_SetChange_1(SYS_CALLBACK_OM_KIND_L3_IF_OPER_STATUS,
            vid_ifindex - SYS_ADPT_VLAN_1_IF_INDEX_NUMBER + 1) == TRUE)
    {
        return SYS_CALLBACK_MGR_NotifyCmgrGroup();
    }
    else
    {
        return FALSE;
    }
#else
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_IfOperStatusChanged_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_IfOperStatusChanged_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_IfOperStatusChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_IfOperStatusChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_L3IF_OPER_STATUS_CHANGED;
    cbdata_msg_p->vid_ifindex = vid_ifindex;
    cbdata_msg_p->oper_status = oper_status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_VlanAddFirstTrunkMemberCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when the first member port is added to a
 *           trunk port.
 *
 * INPUT   : src_csc_id       -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           dot1q_vlan_index -- specify which vlan the member_ifindex port joined
 *           trunk_ifindex    -- specify the trunk port ifindex
 *           member_ifindex   -- specify the member port that joined trunking
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_VlanAddFirstTrunkMemberCallback(UI32_T src_csc_id,
                                                        UI32_T dot1q_vlan_index,
                                                        UI32_T trunk_ifindex,
                                                        UI32_T member_ifindex)
{
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_VlanAddFirstTrunkMember_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanAddFirstTrunkMember_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanAddFirstTrunkMember_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_VlanAddFirstTrunkMember_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_ADD_FIRST_TRUNK_MEMBER;
    cbdata_msg_p->dot1q_vlan_index = dot1q_vlan_index;
    cbdata_msg_p->trunk_ifindex = trunk_ifindex;
    cbdata_msg_p->member_ifindex = member_ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_VlanAddTrunkMemberCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when member port is added to a trunk port.
 *
 * INPUT   : src_csc_id       -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           dot1q_vlan_index -- specify which vlan the member_ifindex port joined
 *           trunk_ifindex    -- specify the trunk port ifindex
 *           member_ifindex   -- specify the member port that joined trunking
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_VlanAddTrunkMemberCallback(UI32_T src_csc_id,
                                                   UI32_T dot1q_vlan_index,
                                                   UI32_T trunk_ifindex,
                                                   UI32_T member_ifindex)
{
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_VlanAddTrunkMember_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanAddTrunkMember_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanAddTrunkMember_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_VlanAddTrunkMember_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_ADD_TRUNK_MEMBER;
    cbdata_msg_p->dot1q_vlan_index = dot1q_vlan_index;
    cbdata_msg_p->trunk_ifindex = trunk_ifindex;
    cbdata_msg_p->member_ifindex = member_ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_VlanDeleteTrunkMemberCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when a member port is deleted from a trunk
 *           port.
 *
 * INPUT   : src_csc_id       -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           dot1q_vlan_index -- specify which vlan the member_ifindex port disjoined
 *           trunk_ifindex    -- specify the trunk port ifindex
 *           member_ifindex   -- specify the member port that is going to be
 *                               remove from the trunk
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_VlanDeleteTrunkMemberCallback(UI32_T src_csc_id,
                                                      UI32_T dot1q_vlan_index,
                                                      UI32_T trunk_ifindex,
                                                      UI32_T member_ifindex)
{
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_VlanDeleteTrunkMember_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanDeleteTrunkMember_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanDeleteTrunkMember_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_VlanDeleteTrunkMember_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_DELETE_TRUNK_MEMBER;
    cbdata_msg_p->dot1q_vlan_index = dot1q_vlan_index;
    cbdata_msg_p->trunk_ifindex = trunk_ifindex;
    cbdata_msg_p->member_ifindex = member_ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_VlanDeleteLastTrunkMemberCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when the last member port is remove from
 *           the trunk port.
 *
 * INPUT   : src_csc_id       -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           dot1q_vlan_index -- specify which vlan the member_ifindex port disjoined
 *           trunk_ifindex    -- specify the trunk port ifindex
 *           member_ifindex   -- specify the member port that is going to be
 *                               remove from the trunk
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_VlanDeleteLastTrunkMemberCallback(UI32_T src_csc_id,
                                                          UI32_T dot1q_vlan_index,
                                                          UI32_T trunk_ifindex,
                                                          UI32_T member_ifindex)
{
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_VlanDeleteLastTrunkMember_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanDeleteLastTrunkMember_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanDeleteLastTrunkMember_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_VlanDeleteLastTrunkMember_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_DELETE_LAST_TRUNK_MEMBER;
    cbdata_msg_p->dot1q_vlan_index = dot1q_vlan_index;
    cbdata_msg_p->trunk_ifindex = trunk_ifindex;
    cbdata_msg_p->member_ifindex = member_ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_VlanFinishAddFirstTrunkMemberCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when the first member port is added to a
 *           trunk port.
 *
 * INPUT   : src_csc_id       -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           dot1q_vlan_index -- specify which vlan the member_ifindex port joined
 *           trunk_ifindex    -- specify the trunk port ifindex
 *           member_ifindex   -- specify the member port that joined trunking
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_VlanFinishAddFirstTrunkMemberCallback(UI32_T src_csc_id,
                                                              UI32_T trunk_ifindex,
                                                              UI32_T member_ifindex)
{
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_VlanFinishAddFirstTrunkMember_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanFinishAddFirstTrunkMember_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanFinishAddFirstTrunkMember_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_VlanFinishAddFirstTrunkMember_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_ADD_FIRST_TRUNK_MEMBER;
    cbdata_msg_p->trunk_ifindex = trunk_ifindex;
    cbdata_msg_p->member_ifindex = member_ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_VlanFinishAddTrunkMemberCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when member port is added to a trunk port.
 *
 * INPUT   : src_csc_id       -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           dot1q_vlan_index -- specify which vlan the member_ifindex port joined
 *           trunk_ifindex    -- specify the trunk port ifindex
 *           member_ifindex   -- specify the member port that joined trunking
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_VlanFinishAddTrunkMemberCallback(UI32_T src_csc_id,
                                                         UI32_T trunk_ifindex,
                                                         UI32_T member_ifindex)
{
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_VlanFinishAddTrunkMember_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanFinishAddTrunkMember_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanFinishAddTrunkMember_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_VlanFinishAddTrunkMember_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_ADD_TRUNK_MEMBER;
    cbdata_msg_p->trunk_ifindex = trunk_ifindex;
    cbdata_msg_p->member_ifindex = member_ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_VlanFinishDeleteTrunkMemberCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when a member port is deleted from a trunk
 *           port.
 *
 * INPUT   : src_csc_id       -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           dot1q_vlan_index -- specify which vlan the member_ifindex port disjoined
 *           trunk_ifindex    -- specify the trunk port ifindex
 *           member_ifindex   -- specify the member port that is going to be
 *                               remove from the trunk
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_VlanFinishDeleteTrunkMemberCallback(UI32_T src_csc_id,
                                                            UI32_T trunk_ifindex,
                                                            UI32_T member_ifindex)
{
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_VlanFinishDeleteTrunkMember_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanFinishDeleteTrunkMember_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanFinishDeleteTrunkMember_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_VlanFinishDeleteTrunkMember_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_DELETE_TRUNK_MEMBER;
    cbdata_msg_p->trunk_ifindex = trunk_ifindex;
    cbdata_msg_p->member_ifindex = member_ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_VlanFinishDeleteLastTrunkMemberCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when the last member port is remove from
 *           the trunk port.
 *
 * INPUT   : src_csc_id       -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           dot1q_vlan_index -- specify which vlan the member_ifindex port disjoined
 *           trunk_ifindex    -- specify the trunk port ifindex
 *           member_ifindex   -- specify the member port that is going to be
 *                               remove from the trunk
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_VlanFinishDeleteLastTrunkMemberCallback(UI32_T src_csc_id,
                                                                UI32_T trunk_ifindex,
                                                                UI32_T member_ifindex)
{
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_VlanFinishDeleteLastTrunkMember_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanFinishDeleteLastTrunkMember_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanFinishDeleteLastTrunkMember_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_VlanFinishDeleteLastTrunkMember_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_DELETE_LAST_TRUNK_MEMBER;
    cbdata_msg_p->trunk_ifindex = trunk_ifindex;
    cbdata_msg_p->member_ifindex = member_ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_VlanPortModeCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when vlan port mode has been changed to
 *           access mode.
 *
 * INPUT   : src_csc_id     -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           lport_ifindex  -- the specific lport to be notify
 *           vlan_port_mode -- vlan_port_mode - value of this field after modification
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_VlanPortModeCallback(UI32_T src_csc_id,
                                             UI32_T lport_ifindex,
                                             UI32_T vlan_port_mode)
{
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    if (SYS_CALLBACK_OM_SetChange_1(SYS_CALLBACK_OM_KIND_PORT_VLAN_MODE,
            lport_ifindex) == TRUE)
    {
        return SYS_CALLBACK_MGR_NotifyCmgrGroup();
    }
    else
    {
        return FALSE;
    }
#else
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_VlanPortMode_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanPortMode_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanPortMode_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_VlanPortMode_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_PORT_MODE;
    cbdata_msg_p->lport_ifindex = lport_ifindex;
    cbdata_msg_p->vlan_port_mode = vlan_port_mode;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_VlanMemberDeleteByTrunkCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : The registered function would be called when a port disjoins a
 *           VLAN by the reason that it joins a trunk.
 *
 * INPUT   : src_csc_id    -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           vid_ifindex   -- specify which vlan's member set to delete from
 *           lport_ifindex -- sepcify which lport to delete from the member set
 *           vlan_status   -- VAL_dot1qVlanStatus_other \
 *                            VAL_dot1qVlanStatus_permanent \
 *                            VAL_dot1qVlanStatus_dynamicGvrp
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_VlanMemberDeleteByTrunkCallback(UI32_T src_csc_id,
                                                        UI32_T vid_ifindex,
                                                        UI32_T lport_ifindex,
                                                        UI32_T vlan_status)
{
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_VlanMemberDeleteByTrunk_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanMemberDeleteByTrunk_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanMemberDeleteByTrunk_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_VlanMemberDeleteByTrunk_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE_BY_TRUNK;
    cbdata_msg_p->vid_ifindex = vid_ifindex;
    cbdata_msg_p->lport_ifindex = lport_ifindex;
    cbdata_msg_p->vlan_status = vlan_status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}


/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_VlanNameChangedCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : The registered function would be called when a vlan's name has
 *           been changed.
 *
 * INPUT   : src_csc_id -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           vid        -- specify the id of vlan whose name has been changed
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_VlanNameChangedCallback(UI32_T src_csc_id, UI32_T vid)
{
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    if (SYS_CALLBACK_OM_SetChange_1(SYS_CALLBACK_OM_KIND_VLAN_NAME, vid) ==
            TRUE)
    {
        return SYS_CALLBACK_MGR_NotifyCmgrGroup();
    }
    else
    {
        return FALSE;
    }
#else
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_VlanNameChanged_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanNameChanged_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanNameChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_VlanNameChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_NAME_CHANGED;
    cbdata_msg_p->vid = vid;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_ProtoVlanGroupIdBindingChangedCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : The registered function would be called when the protocol vlan
 *           group id binding for a port has been changed.
 *           been changed.
 *
 * INPUT   : src_csc_id -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           lport      -- specify lport whose protocol group id binding has
 *                         been changed
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ProtoVlanGroupIdBindingChangedCallback(UI32_T src_csc_id, UI32_T lport)
{
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    if (SYS_CALLBACK_OM_SetChange_1(SYS_CALLBACK_OM_KIND_PROTOCOL_VLAN, lport)
            == TRUE)
    {
        return SYS_CALLBACK_MGR_NotifyCmgrGroup();
    }
    else
    {
        return FALSE;
    }
#else
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_ProtovlanGidBindingChanged_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_ProtovlanGidBindingChanged_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_ProtovlanGidBindingChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_ProtovlanGidBindingChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_PROTOVLAN_GID_BINDING_CHANGED;
    cbdata_msg_p->lport = lport;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_VlanMemberTagChangedCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : The registered function would be called when tag type of a port
 *           member for a VLAN is changed.
 *
 * INPUT   : src_csc_id    -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           vid_ifindex   -- the ifindex of the VLAN
 *           lport_ifindex -- the ifindex the port
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : Untagged member -> Tagged member / Tagged member -> Untagged member
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_VlanMemberTagChangedCallback(UI32_T src_csc_id,
                                                     UI32_T vid_ifindex,
                                                     UI32_T lport_ifindex)
{
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    if (SYS_CALLBACK_OM_SetChange_2(SYS_CALLBACK_OM_KIND_VLAN_MEMBER_TAG,
            vid_ifindex - SYS_ADPT_VLAN_1_IF_INDEX_NUMBER + 1, lport_ifindex) ==
            TRUE)
    {
        return SYS_CALLBACK_MGR_NotifyCmgrGroup();
    }
    else
    {
        return FALSE;
    }
#else
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T *syscb_msg_p;
    SYS_CALLBACK_MGR_VlanMemberTagChanged_CBData_T *cbdata_msg_p;
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanMemberTagChanged_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_VlanMemberTagChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_VlanMemberTagChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_TAG_CHANGED;
    cbdata_msg_p->vid_ifindex = vid_ifindex;
    cbdata_msg_p->lport_ifindex = lport_ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
#endif
}

/*********************************
 *              XSTP             *
 *********************************
 */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_LportEnterForwardingCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : When a port enters the forwarding state, XSTP will notify other
 *           CSC groups by this function.
 *
 * INPUT   : src_csc_id -- The csc_id who triggers this event (SYS_MODULE_XSTP)
 *           xstid      -- index of the spanning tree
 *           lport      -- logical port number
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_LportEnterForwardingCallback(UI32_T src_csc_id,
                                                     UI32_T xstid,
                                                     UI32_T lport)
{
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    if (SYS_CALLBACK_OM_SetChange_3(SYS_CALLBACK_OM_KIND_XSTP_PORT_STATE,
            XSTP_POM_GetInstanceEntryId(xstid), lport,
            SYS_CALLBACK_OM_ACTION_TO_ON) == TRUE)
    {
        return SYS_CALLBACK_MGR_NotifyCmgrGroup();
    }
    else
    {
        return FALSE;
    }
#else
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_LportEnterForwarding_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LportEnterForwarding_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LportEnterForwarding_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LportEnterForwarding_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_LPORT_ENTER_FORWARDING;
    cbdata_msg_p->xstid = xstid;
    cbdata_msg_p->lport = lport;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_LportLeaveForwardingCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : When a port leaves the forwarding state, XSTP will notify other
 *           CSC groups by this function.
 *
 * INPUT   : src_csc_id -- The csc_id who triggers this event (SYS_MODULE_XSTP)
 *           xstid      -- index of the spanning tree
 *           lport      -- logical port number
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_LportLeaveForwardingCallback(UI32_T src_csc_id,
                                                     UI32_T xstid,
                                                     UI32_T lport)
{
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    if (SYS_CALLBACK_OM_SetChange_3(SYS_CALLBACK_OM_KIND_XSTP_PORT_STATE,
            XSTP_POM_GetInstanceEntryId(xstid), lport,
            SYS_CALLBACK_OM_ACTION_TO_OFF) == TRUE)
    {
        return SYS_CALLBACK_MGR_NotifyCmgrGroup();
    }
    else
    {
        return FALSE;
    }
#else
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_LportLeaveForwarding_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LportLeaveForwarding_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LportLeaveForwarding_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LportLeaveForwarding_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_LPORT_LEAVE_FORWARDING;
    cbdata_msg_p->xstid = xstid;
    cbdata_msg_p->lport = lport;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_LportChangeStateCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : When any port enters/leaves the forwarding state, XSTP will notify
 *           other CSC groups by this function.
 *
 * INPUT   : src_csc_id -- The csc_id who triggers this event (SYS_MODULE_XSTP)
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_LportChangeStateCallback(UI32_T src_csc_id)
{
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_Msg_T))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = sizeof(SYS_CALLBACK_MGR_Msg_T);
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_LPORT_CHANGE_STATE;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}
/*add by Tony.Lei for IGMPSnooping */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_LportTcChangeCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : When any port enters tc change, XSTP will notify
 *           IGMPSnooping  by this function.
 *
 * INPUT   : src_csc_id -- The csc_id who triggers this event (SYS_MODULE_XSTP)
 *                   xstp_mode, xstid,lport,is_root,tc_timer
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_LportTcChangeCallback(UI32_T src_csc_id,
        BOOL_T is_mstp_mode, UI32_T xstid, UI32_T lport, BOOL_T is_root,
        UI32_T tc_timer, UI8_T *vlan_bit_map)
{
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    if (SYS_CALLBACK_OM_SetChange_2(SYS_CALLBACK_OM_KIND_XSTP_PORT_TOPO,
            XSTP_POM_GetInstanceEntryId(xstid), lport) == TRUE)
    {
        return SYS_CALLBACK_MGR_NotifyCmgrGroup();
    }
    else
    {
        return FALSE;
    }
#else
        SYSFUN_Msg_T    *sysfun_msg_p;
        SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
        SYS_CALLBACK_MGR_LportTcChange_CBData_T *cbdata_msg_p;
        UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LportTcChange_CBData_T)))];

        sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
        sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LportTcChange_CBData_T));
        syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
        cbdata_msg_p= (SYS_CALLBACK_MGR_LportTcChange_CBData_T*)syscb_msg_p->callback_data;

        syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_XSTP_LPORT_TC_CHANGE;
        cbdata_msg_p->is_mstp_mode = is_mstp_mode;
        cbdata_msg_p->xstid = xstid;
        cbdata_msg_p->lport = lport;
        cbdata_msg_p->is_root = is_root;
        cbdata_msg_p->tc_timer = tc_timer;

        /* to reduce the message size, the vlan bitmap will be retrieved from the
         *  the xstid inside the api but not passed by parameter.
         *
         * Ryan agrees to take the risk that the vlan bitmap retrieved may be not
         *  the same as the one when syscallback is called.
         */
//      memcpy(cbdata_msg_p->instance_vlans_mapped,vlan_bit_map,512);

        return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p,FALSE);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_StpChangeVersionCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : When any port enters/leaves the forwarding state, XSTP will notify
 *           other CSC groups by this function.
 *
 * INPUT   : src_csc_id -- The csc_id who triggers this event (SYS_MODULE_XSTP)
 *           mode       -- current spanning tree mode
 *           status     -- current spanning tree status
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_StpChangeVersionCallback(UI32_T src_csc_id,
                                                 UI32_T mode, UI32_T status)
{
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    if (SYS_CALLBACK_OM_SetChange_0(SYS_CALLBACK_OM_KIND_XSTP_VERSION) == TRUE)
    {
        return SYS_CALLBACK_MGR_NotifyCmgrGroup();
    }
    else
    {
        return FALSE;
    }
#else
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_StpChangeVersion_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_StpChangeVersion_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_StpChangeVersion_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_StpChangeVersion_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_STP_CHANGE_VERSION;
    cbdata_msg_p->mode = mode;
    cbdata_msg_p->status = status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
#endif
}

/*********************************
 *           SWCTRL              *
 *********************************
 */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Notify_PortOperUpCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the oper status is up
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_PortOperUpCallback(
    UI32_T src_csc_id,UI32_T ifindex)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_LPort_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_LPort_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LPort_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LPort_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_OPER_UP;
    cbdata_msg_p->ifindex = ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Notify_PortNotOperUPCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the oper status is not up
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_PortNotOperUpCallback(
    UI32_T src_csc_id,UI32_T ifindex)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_LPort_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_LPort_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LPort_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LPort_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_NOT_OPER_UP;
    cbdata_msg_p->ifindex = ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_PortAdminEnableCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the port is enabled
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_PortAdminEnableCallback(
    UI32_T src_csc_id,UI32_T ifindex)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_LPort_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_LPort_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LPort_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LPort_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_ADMIN_ENABLE;
    cbdata_msg_p->ifindex = ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_PortAdminDisableCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the port is disabled
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_PortAdminDisableCallback(
    UI32_T src_csc_id,UI32_T ifindex)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_LPort_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_LPort_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LPort_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LPort_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_ADMIN_DISABLE;
    cbdata_msg_p->ifindex = ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_PortSpeedDuplexCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the speed/duplex status of a port
 *           is changed
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           ifindex -- which logical port
 *           speed_duplex -- new status of speed/duplex
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_PortSpeedDuplexCallback(
    UI32_T src_csc_id,UI32_T ifindex,UI32_T speed_duplex)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_LPortSpeedDuplex_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_LPortSpeedDuplex_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LPortSpeedDuplex_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LPortSpeedDuplex_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_SPEED_DUPLEX;
    cbdata_msg_p->ifindex = ifindex;
    cbdata_msg_p->speed_duplex = speed_duplex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_TrunkMemberAdd1stCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the first port is added to a
 *           trunk.
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_TrunkMemberAdd1stCallback(
    UI32_T src_csc_id,UI32_T trunk_ifindex,UI32_T member_ifindex)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_TrunkMember_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_TrunkMember_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_TrunkMember_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_TrunkMember_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD_1ST;
    cbdata_msg_p->trunk_ifindex = trunk_ifindex;
    cbdata_msg_p->member_ifindex = member_ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_TrunkMemberAddCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when a logical port is added to a
 *           trunk
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_TrunkMemberAddCallback(
    UI32_T src_csc_id,UI32_T trunk_ifindex,UI32_T member_ifindex)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_TrunkMember_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_TrunkMember_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_TrunkMember_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_TrunkMember_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD;
    cbdata_msg_p->trunk_ifindex = trunk_ifindex;
    cbdata_msg_p->member_ifindex = member_ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_TrunkMemberDeleteCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when a logical port is deleted from a
 *           trunk
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_TrunkMemberDeleteCallback(
    UI32_T src_csc_id,UI32_T trunk_ifindex,UI32_T member_ifindex)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_TrunkMember_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_TrunkMember_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_TrunkMember_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_TrunkMember_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE;
    cbdata_msg_p->trunk_ifindex = trunk_ifindex;
    cbdata_msg_p->member_ifindex = member_ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_TrunkMemberDeleteLstCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when a logical port is deleted from a
 *           trunk
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_TrunkMemberDeleteLstCallback(
    UI32_T src_csc_id,UI32_T trunk_ifindex,UI32_T member_ifindex)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_TrunkMember_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_TrunkMember_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_TrunkMember_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_TrunkMember_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE_LST;
    cbdata_msg_p->trunk_ifindex = trunk_ifindex;
    cbdata_msg_p->member_ifindex = member_ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Notify_TrunkMemberPortOperUpCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the oper status is up
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_TrunkMemberPortOperUpCallback(
    UI32_T src_csc_id,UI32_T trunk_ifindex,UI32_T trunk_member_port_ifindex)
{

    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_TrunkMember_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_TrunkMember_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_TrunkMember_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_TrunkMember_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_PORT_OPER_UP;
    cbdata_msg_p->trunk_ifindex = trunk_ifindex;
    cbdata_msg_p->member_ifindex = trunk_member_port_ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Notify_TrunkMemberPortNotOperUpCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the oper status is down
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_TrunkMemberPortNotOperUpCallback(
    UI32_T src_csc_id,UI32_T trunk_ifindex,UI32_T trunk_member_port_ifindex)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_TrunkMember_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_TrunkMember_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_TrunkMember_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_TrunkMember_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_PORT_NOT_OPER_UP;
    cbdata_msg_p->trunk_ifindex = trunk_ifindex;
    cbdata_msg_p->member_ifindex = trunk_member_port_ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_TrunkMemberActiveCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when member port is active
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_TrunkMemberActiveCallback(
    UI32_T src_csc_id,UI32_T trunk_ifindex,UI32_T trunk_member_port_ifindex)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_TrunkMember_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_TrunkMember_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_TrunkMember_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_TrunkMember_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ACTIVE;
    cbdata_msg_p->trunk_ifindex = trunk_ifindex;
    cbdata_msg_p->member_ifindex = trunk_member_port_ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_TrunkMemberInactiveCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when member port is inactive
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_TrunkMemberInactiveCallback(
    UI32_T src_csc_id,UI32_T trunk_ifindex,UI32_T trunk_member_port_ifindex)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_TrunkMember_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_TrunkMember_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_TrunkMember_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_TrunkMember_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_INACTIVE;
    cbdata_msg_p->trunk_ifindex = trunk_ifindex;
    cbdata_msg_p->member_ifindex = trunk_member_port_ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_uPortSpeedDuplexCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the speed/duplex status of a port
 *           is changed
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           unit -- in which unit
 *           port -- which logical port
 *           speed_duplex -- new status of speed/duplex
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_uPortSpeedDuplexCallback(
    UI32_T src_csc_id,UI32_T unit,UI32_T port,UI32_T speed_duplex)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_UPortSpeedDuplex_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_UPortSpeedDuplex_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_UPortSpeedDuplex_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_UPortSpeedDuplex_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_SPEED_DUPLEX;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;
    cbdata_msg_p->speed_duplex = speed_duplex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_uPortLinkUpCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the link is up
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           unit -- in which unit
 *           port -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_uPortLinkUpCallback(
    UI32_T src_csc_id,UI32_T unit,UI32_T port)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_UPort_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_UPort_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_UPort_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_UPort_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_LINK_UP;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Notify_uPortLinkDownCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the link is down
 * INPUT   :src_csc_id       -- The csc_id who triggers this event
 *            unit -- in which unit
 *           port -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_uPortLinkDownCallback(
    UI32_T src_csc_id,UI32_T unit,UI32_T port)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_UPort_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_UPort_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_UPort_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_UPort_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_LINK_DOWN;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_uPortAdminEnableCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the port is enabled
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           unit -- in which unit
 *           port -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_uPortAdminEnableCallback(
    UI32_T src_csc_id,UI32_T unit,UI32_T port)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_UPort_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_UPort_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_UPort_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_UPort_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_ADMIN_ENABLE;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_uPortAdminDisableCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the port is disabled
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           unit -- in which unit
 *           port -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_uPortAdminDisableCallback(
    UI32_T src_csc_id,UI32_T unit,UI32_T port)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_UPort_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_UPort_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_UPort_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_UPort_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_ADMIN_DISABLE;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_PortStatusChangedPassivelyCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the port status is changed passively
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           ifindex -- which logical port
 *           status
 *           changed_bmp
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_PortStatusChangedPassivelyCallback(
    UI32_T src_csc_id, UI32_T ifindex, BOOL_T status, UI32_T changed_bmp)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_LPortStatus_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_LPortStatus_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LPortStatus_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LPortStatus_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_STATUS_CHANGED_PASSIVELY;
    cbdata_msg_p->ifindex = ifindex;
    cbdata_msg_p->status_bool = status;
    cbdata_msg_p->status_u32 = changed_bmp;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Notify_PortLinkDownCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the link is down
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_PortLinkDownCallback(
    UI32_T src_csc_id,UI32_T ifindex)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_LPort_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_LPort_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LPort_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LPort_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_LINK_DOWN;
    cbdata_msg_p->ifindex = ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_PortLinkUpCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the link is up
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_PortLinkUpCallback(
    UI32_T src_csc_id,UI32_T ifindex)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_LPort_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_LPort_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LPort_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LPort_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_LINK_UP;
    cbdata_msg_p->ifindex = ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_uPortFastLinkUpCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the link is up
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           unit -- in which unit
 *           port -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_uPortFastLinkUpCallback(
    UI32_T src_csc_id,UI32_T unit,UI32_T port)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_UPort_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_UPort_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_UPort_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_UPort_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_FAST_LINK_UP;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Notify_uPortFastLinkDownCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the link is down
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           unit -- in which unit
 *           port -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_uPortFastLinkDownCallback(
    UI32_T src_csc_id,UI32_T unit,UI32_T port)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_UPort_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_UPort_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_UPort_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_UPort_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_FAST_LINK_DOWN;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_PortAdminDisableBeforeCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the port is disabled
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : for notify LLDP before doing shutdown port
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_PortAdminDisableBeforeCallback(
    UI32_T src_csc_id,UI32_T ifindex)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_LPort_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_LPort_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LPort_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LPort_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_ADMIN_DISABLE_BEFORE;
    cbdata_msg_p->ifindex = ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_uPortLacpEffectiveOperStatusChangedCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the effective oper status change
 *           for LACP.
 *           is changed
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           unit           --- which unit
 *           port           --- which port
 *           pre_status     --- status before change
 *           current_status --- status after change
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_uPortLacpEffectiveOperStatusChangedCallback(
    UI32_T src_csc_id,UI32_T unit,UI32_T port,UI32_T pre_status, UI32_T current_status )
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_uPortLacpEffectiveOperStatus_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_uPortLacpEffectiveOperStatus_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_uPortLacpEffectiveOperStatus_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_uPortLacpEffectiveOperStatus_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_LACP_EFFECTIVE_OPER_STATUS_CHANGED;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;
    cbdata_msg_p->pre_status = pre_status;
    cbdata_msg_p->current_status = current_status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_uPortDot1xEffectiveOperStatusChangedCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the effective oper status change
 *           for DOT1x.
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           unit           --- which unit
 *           port           --- which port
 *           pre_status     --- status before change
 *           current_status --- status after change
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_uPortDot1xEffectiveOperStatusChangedCallback(
    UI32_T src_csc_id,UI32_T unit, UI32_T port,UI32_T pre_status, UI32_T current_status )
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_uPortLacpEffectiveOperStatus_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_uPortLacpEffectiveOperStatus_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_uPortLacpEffectiveOperStatus_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_uPortLacpEffectiveOperStatus_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_DOT1X_EFFECTIVE_OPER_STATUS_CHANGED;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;
    cbdata_msg_p->pre_status = pre_status;
    cbdata_msg_p->current_status = current_status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_PortEffectiveOperStatusChangedCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the effective oper status change.
 * INPUT   : ifindex        --- which ifindex
 *           pre_status     --- status before change
 *           current_status --- status after change
 *           level          --- dormant level after change
 *                              see SWCTRL_OperDormantLevel_T
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : When to callback:
 *           1) Oper status becomes effecitve.
 *              Oper status is changed from lower status to specified dormant
 *              status.
 *           2) Oper status becomes ineffecitve.
 *              Oper status is changed from specified dormant status or upper
 *              status to lower status.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_PortEffectiveOperStatusChangedCallback(
    UI32_T src_csc_id,
    UI32_T dest_msgq,
    UI32_T ifindex,
    UI32_T pre_status,
    UI32_T current_status,
    UI32_T level)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_PortEffectiveOperStatus_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_PortEffectiveOperStatus_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_PortEffectiveOperStatus_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_PortEffectiveOperStatus_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_EFFECTIVE_OPER_STATUS_CHANGED;
    cbdata_msg_p->ifindex = ifindex;
    cbdata_msg_p->pre_status = pre_status;
    cbdata_msg_p->current_status = current_status;
    cbdata_msg_p->level = level;

    return SYS_CALLBACK_MGR_SendMsgWithMsgQKey(src_csc_id, dest_msgq, sysfun_msg_p, FALSE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_ForwardingUPortAddToTrunkCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when forwarding uport added to trunk.
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           trunk_ifindex  - Which trunk.
 *           member_ifindex - Which trunk member.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_ForwardingUPortAddToTrunkCallback(
    UI32_T src_csc_id,UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_TrunkMember_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_TrunkMember_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_TrunkMember_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_TrunkMember_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_FORWARDING_UPORT_ADD_TO_TRUNK;
    cbdata_msg_p->trunk_ifindex = trunk_ifindex;
    cbdata_msg_p->member_ifindex = member_ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_ForwardingTrunkMemberDeleteCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when forwarding trunk member deleted.
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           trunk_ifindex  - Which trunk.
 *           member_ifindex - Which trunk member.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_ForwardingTrunkMemberDeleteCallback(
    UI32_T src_csc_id,UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_TrunkMember_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_TrunkMember_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_TrunkMember_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_TrunkMember_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_FORWARDING_TRUNK_MEMBER_DELETE;
    cbdata_msg_p->trunk_ifindex = trunk_ifindex;
    cbdata_msg_p->member_ifindex = member_ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_ForwardingTrunkMemberToNonForwardingCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when forwarding trunk member become non-forwarding.
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           trunk_ifindex  - Which trunk.
 *           member_ifindex - Which trunk member.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_ForwardingTrunkMemberToNonForwardingCallback(
    UI32_T src_csc_id,UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_TrunkMember_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_TrunkMember_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_TrunkMember_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_TrunkMember_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_FORWARDING_TRUNK_MEMBER_TO_NON_FORWARDING;
    cbdata_msg_p->trunk_ifindex = trunk_ifindex;
    cbdata_msg_p->member_ifindex = member_ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_LPortTypeChangedCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when a hot swap module is removed.
 * INPUT   : src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *           ifindex   -- which logical port.
 *           port_type -- changed to which port type.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/

BOOL_T SYS_CALLBACK_MGR_LPortTypeChangedCallback(
    UI32_T src_csc_id,UI32_T ifindex, UI32_T port_type)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_LPortType_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_LPortType_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LPortType_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LPortType_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_LPORT_TYPE_CHANGED;
    cbdata_msg_p->ifindex = ifindex;
    cbdata_msg_p->port_type = port_type;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_UPortTypeChangedCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when a hot swap module is removed.
 * INPUT   : src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *           ifindex   -- which logical port.
 *           port_type -- changed to which port type.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_UPortTypeChangedCallback(
    UI32_T src_csc_id,UI32_T unit, UI32_T port, UI32_T port_type)
{

    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_UPortType_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_UPortType_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_UPortType_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_UPortType_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_TYPE_CHANGED;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;
    cbdata_msg_p->port_type = port_type;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_IfMauChangedCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the MAU of a port has been changed.
 * INPUT   : src_csc_id -- The csc_id who triggers this event (SYS_MODULE_SWCTRL)
 *           lport      -- which logical port.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_IfMauChangedCallback(UI32_T src_csc_id, UI32_T lport)
{

    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_IfMauChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_IfMauChanged_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_IfMauChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_IfMauChanged_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_IF_MAU_CHANGED;
    cbdata_msg_p->lport = lport;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_PortLearningStatusChangedCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when learning status of a port has been changed.
 * INPUT   : src_csc_id -- The csc_id who triggers this event (SYS_MODULE_SWCTRL)
 *           lport      -- which logical port.
 *           learning
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_PortLearningStatusChangedCallback(UI32_T src_csc_id, UI32_T lport, BOOL_T learning)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_PortLearningStatusChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_PortLearningStatusChanged_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_PortLearningStatusChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_PortLearningStatusChanged_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_LEARNING_STATUS_CHANGED;
    cbdata_msg_p->lport = lport;
    cbdata_msg_p->learning = learning;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}


/*********************************
 *            AMTRDRV            *
 *********************************
 */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_NewMacAddress
 * -------------------------------------------------------------------------
 * FUNCTION: AMTRdrv announce NA to AMTR (MAC, VID, Port)
 * INPUT   : src_csc_id         -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
             addr_buf.vid       -- which VID number
 *           addr_buf.mac       -- what's the mac address
 *           addr_buf.ifindex   -- which unit which port or which trunk_id
 *           num_of_entries     -- number of entry
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_AnnounceNewMacAddress(UI32_T src_csc_id, UI32_T num_of_entries, UI8_T *addr_buf, UI32_T buf_size)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_NewMacAddress_CBData_T  *cbdata_msg_p;
    UI8_T        *ipcmsg_buf;
    BOOL_T       return_value;
    UI32_T       total_msg_size;

    total_msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_NewMacAddress_CBData_T))+buf_size;
    ipcmsg_buf = L_MM_Malloc(SYSFUN_SIZE_OF_MSG(total_msg_size),L_MM_USER_ID2(SYS_MODULE_SYS_CALLBACK, EXT_TRACE_ID_ANNOUNCENEWMACADDRESS));
    if (ipcmsg_buf == NULL)
    {
        return FALSE;
    }
    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = total_msg_size;
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_NewMacAddress_CBData_T*)syscb_msg_p->callback_data;
    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_NEW_MAC_ADDRESS;
    cbdata_msg_p->number_of_entry = num_of_entries;
    memcpy(cbdata_msg_p->addr_buf,addr_buf, buf_size);
    return_value = SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
    L_MM_Free(ipcmsg_buf);
    return return_value;
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_AgeOutMacAddress
 * -------------------------------------------------------------------------
 * FUNCTION: AMTRdrv announce NA to AMTR (MAC, VID, Port)
 * INPUT   : src_csc_id         -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *           addr_buf.vid       -- which VID number
 *           addr_buf.mac       -- what's the mac address
 *           addr_buf.ifindex   -- which unit which port or which trunk_id
 *           num_of_entries     -- number of entry
 *           buf_size           -- num_of_entries x sizeof(AMTR_TYPE_AddrEntry_T)
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_AgeOutMacAddress(UI32_T src_csc_id, UI32_T num_of_entries, UI8_T *addr_buf, UI32_T buf_size)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_NewMacAddress_CBData_T  *cbdata_msg_p;
    UI8_T        *ipcmsg_buf;
    BOOL_T       return_value;
    UI32_T       total_msg_size;

    total_msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_NewMacAddress_CBData_T))+buf_size;
    ipcmsg_buf = L_MM_Malloc(SYSFUN_SIZE_OF_MSG(total_msg_size),L_MM_USER_ID2(SYS_MODULE_SYS_CALLBACK, EXT_TRACE_ID_AGEOUTMACADDRESS));
    if (ipcmsg_buf == NULL)
    {
        return FALSE;
    }
    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = total_msg_size;
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_NewMacAddress_CBData_T*)syscb_msg_p->callback_data;
    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_AGING_OUT;
    cbdata_msg_p->number_of_entry = num_of_entries;
    memcpy(cbdata_msg_p->addr_buf,addr_buf, buf_size);
    return_value = SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
    L_MM_Free(ipcmsg_buf);
    return return_value;
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_SecurityCheckCallback
 * -------------------------------------------------------------------------
 * FUNCTION: This API will check intrusion and port move.
 * INPUT   : src_csc_id    -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *           src_lport     -- source lport
 *           vid           -- which VID number
 *           src_mac       -- source mac address
 *           dst_mac       -- destination mac address
 *           ether_type    -- packet type
 * OUTPUT  : None
 * RETURN  : AMTR_TYPE_CHECK_INTRUSION_RETVAL_DROP  -- drop
 *           AMTR_TYPE_CHECK_INTRUSION_RETVAL_LEARN -- learn
 *           0                                      -- no drop & no learn
 * NOTE    : refinement
 *           1.When lan.c receive packet, AMTR have to check whether it's intrusion or not.
 *             Intrusion mac can't be put in NA buffer and can't run procedure about protocol.
 *           2. In Hardware Learning, AMTR notify intrusion mac by this callback function.
 *--------------------------------------------------------------------------*/
UI32_T SYS_CALLBACK_MGR_SecurityCheckCallback(UI32_T src_csc_id, UI32_T src_lport, UI16_T vid, UI8_T *src_mac, UI8_T *dst_mac, UI16_T ether_type)
{
    UI32_T          trunk_ifindex;
    BOOL_T          is_static_trunk;

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
    AMTRDRV_TYPE_Record_T  original_entry;
    BOOL_T is_exist_om = FALSE;

/* remove bcz the check always fail...
 */
#if 0
    if(SYS_CALLBACK_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return AMTR_TYPE_CHECK_INTRUSION_RETVAL_LEARN;
#endif

    /* AMTRDRV can't transform unit+port into trunk ifindex.
     * The addr_buf[entry_index].ifindex only identify ifindex of normal port.
     * In Core Layer, AMTR have to transform ifindex of trunk member into trunk_ifindex.
     * If (SWCTRL_IsTrunkMember() == TRUE), src_lport will be modified to trunk ifindex.
     */
    if (SWCTRL_POM_IsTrunkMember(src_lport, &trunk_ifindex, &is_static_trunk)==TRUE)
    {
        src_lport = trunk_ifindex;
    }

    if(AMTR_PMGR_IsPortSecurityEnabled(src_lport))
    {
        /* learn_with_count is full, no learn.
         */
#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)
        if (AMTR_PMGR_Notify_IntrusionMac(src_lport, vid, src_mac,dst_mac, ether_type))
            return AMTR_TYPE_CHECK_INTRUSION_RETVAL_DROP;   /*no learnt & drop*/
        else
#endif
            return 0;                                       /*no learnt & no drop*/
    }

    original_entry.address.vid=vid;
    memcpy(original_entry.address.mac,src_mac,AMTR_TYPE_MAC_LEN);
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
            AMTR_MGR_PortInfo_T port_info;
            /* port move from secure port, drop & no learn
             */
            if (FALSE == AMTR_POM_GetPortInfo(original_entry.address.ifindex, &port_info))
            {
                SYSFUN_Debug_Printf("%s:AMTR_PMGR_GetPortInfo return false\n",__FUNCTION__);
                return AMTR_TYPE_CHECK_INTRUSION_RETVAL_DROP;  /*no learnt & drop*/
            }
            if (port_info.protocol!=AMTR_MGR_PROTOCOL_NORMAL)
            {
                AMTR_PMGR_Notify_SecurityPortMove(src_lport, original_entry.address.vid, src_mac, original_entry.address.ifindex);
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

/* remove bcz the check always fail...
 */
#if 0
    if(SYS_CALLBACK_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return AMTR_TYPE_CHECK_INTRUSION_RETVAL_LEARN;
    }
#endif

    /* by former implementation, AMTR in hw learn mode
     * takes all received pkt (by this function) as intrusion mac.
     * so always assume security mode is enable.
     *
     * in actual, the pkt may be just a NA pkt.
     * ex. NA pkt trapped with wrong reason (ex. Marvell chip)
     *     -> LAN check AMTRDRV database to determine if it is NA.
     *     -> LAN take is as NA due to AMTR db lookup failed even though HW learn this mac already.
     *        (because MAC table sync is not instant.)
     *     -> when security mode is disabled, do nothing.
     */
    if (!AMTR_PMGR_IsPortSecurityEnabled(src_lport))
    {
        return AMTR_TYPE_CHECK_INTRUSION_RETVAL_LEARN;
    }

    /* 1x packet, not intrusion
     */
    if (0x888e == ether_type)
    {
        return AMTR_TYPE_CHECK_INTRUSION_RETVAL_LEARN;
    }

    if (SWCTRL_POM_IsTrunkMember(src_lport, &trunk_ifindex, &is_static_trunk)==TRUE)
    {
        src_lport = trunk_ifindex;
    }

    /* In HW learning, AMTR notify to PSec if and only if intruction mac.
     * (not ask PSec to do intrusion checking)
     * So, DA = NULL MAC; ether type = 0.
     */
    if (AMTR_PMGR_Notify_IntrusionMac(src_lport, vid, src_mac, dst_mac, ether_type))
        return AMTR_TYPE_CHECK_INTRUSION_RETVAL_DROP;   /*no learnt & drop*/
    else
        return AMTR_TYPE_CHECK_INTRUSION_RETVAL_LEARN;  /*learnt & no drop*/
#endif
}

/*********************************
 *            NMTRDRV            *
 *********************************
 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_UpdateLocalNmtrdrvStats
 * -------------------------------------------------------------------------
 * FUNCTION: AMTRdrv announce NA to AMTR (MAC, VID, Port)
 * INPUT   : src_csc_id  -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *           update_type -- Indicate the type of update
 *           start_port  -- Start port number
 *           port_amount -- Total amount of update ports
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_UpdateNmtrdrvStats(UI32_T src_csc_id, UI32_T update_type, UI32_T unit,UI32_T start_port, UI32_T port_amount)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_UpdateLocalNmtrdrvStats_CBData_T *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_UpdateLocalNmtrdrvStats_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_UpdateLocalNmtrdrvStats_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_UpdateLocalNmtrdrvStats_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_UPDATE_LOCAL_NMTRDRV_STATS;
    cbdata_msg_p->update_type = update_type;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->start_port  = start_port;
    cbdata_msg_p->port_amount = port_amount;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*********************************
 *             SWDRV             *
 *********************************
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_MGR_SWDRV_PortLinkUp
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Notify port link down.
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      unit
 *      port
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_SWDRV_PortLinkUp(UI32_T src_csc_id, UI32_T unit, UI32_T port)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_SWDRV_PortLinkUp_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_PortLinkUp_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_PortLinkUp_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_SWDRV_PortLinkUp_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_LINK_UP;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_MGR_SWDRV_PortLinkDown
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Notify port link down.
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      unit
 *      port
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_SWDRV_PortLinkDown(UI32_T src_csc_id, UI32_T unit, UI32_T port)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_SWDRV_PortLinkDown_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_PortLinkDown_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_PortLinkDown_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_SWDRV_PortLinkDown_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_LINK_DOWN;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_MGR_SWDRV_CraftPortLinkUp
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Notify port link down.
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      unit
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_SWDRV_CraftPortLinkUp(UI32_T src_csc_id, UI32_T unit)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_SWDRV_CraftPortLinkUp_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_CraftPortLinkUp_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_CraftPortLinkUp_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_SWDRV_CraftPortLinkUp_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_CRAFT_PORT_LINK_UP;
    cbdata_msg_p->unit = unit;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_MGR_SWDRV_CraftPortLinkDown
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Notify port link down.
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      unit
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_SWDRV_CraftPortLinkDown(UI32_T src_csc_id, UI32_T unit)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_SWDRV_CraftPortLinkDown_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_CraftPortLinkDown_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_CraftPortLinkDown_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_SWDRV_CraftPortLinkDown_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_CRAFT_PORT_LINK_DOWN;
    cbdata_msg_p->unit = unit;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_MGR_SWDRV_PortTypeChanged
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Notify port link down.
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      unit
 *      port
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_SWDRV_PortTypeChanged(UI32_T src_csc_id,
                                              UI32_T unit,
                                              UI32_T port,
                                              UI32_T module_id,
                                              UI32_T port_type)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_SWDRV_PortTypeChanged_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_PortTypeChanged_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_PortTypeChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_SWDRV_PortTypeChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_TYPE_CHANGED;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;
    cbdata_msg_p->module_id = module_id;
    cbdata_msg_p->port_type = port_type;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_MGR_SWDRV_PortSpeedDuplex
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Notify port link down.
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      unit
 *      port
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_SWDRV_PortSpeedDuplex(UI32_T src_csc_id,
                                              UI32_T unit,
                                              UI32_T port,
                                              UI32_T speed_duplex)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_SWDRV_PortSpeedDuplex_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_PortSpeedDuplex_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_PortSpeedDuplex_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_SWDRV_PortSpeedDuplex_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_SPEED_DUPLEX;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;
    cbdata_msg_p->speed_duplex= speed_duplex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_MGR_SWDRV_PortSpeedDuplex
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Notify port link down.
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      unit
 *      port
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_SWDRV_PortFlowCtrl(UI32_T src_csc_id,
                                           UI32_T unit,
                                           UI32_T port,
                                           UI32_T flow_ctrl)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_SWDRV_PortFlowCtrl_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_PortFlowCtrl_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_PortFlowCtrl_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_SWDRV_PortFlowCtrl_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_FLOW_CTRL;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;
    cbdata_msg_p->flow_ctrl = flow_ctrl;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_MGR_SWDRV_HotSwapInsert
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Notify port link down.
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      unit
 *      port
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_SWDRV_HotSwapInsert(UI32_T src_csc_id, UI32_T unit, UI32_T port)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_SWDRV_HotSwapInsert_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_HotSwapInsert_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_HotSwapInsert_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_SWDRV_HotSwapInsert_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_HOT_SWAP_INSERT;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_MGR_SWDRV_HotSwapRemove
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Notify port link down.
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      unit
 *      port
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_SWDRV_HotSwapRemove(UI32_T src_csc_id, UI32_T unit, UI32_T port)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_SWDRV_HotSwapRemove_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_HotSwapRemove_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_HotSwapRemove_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_SWDRV_HotSwapRemove_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_HOT_SWAP_REMOVE;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_MGR_SWDRV_PortSfpPresent
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Notify port sfp is present or not.
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      unit
 *      port
 *      is_present
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_SWDRV_PortSfpPresent(UI32_T src_csc_id, UI32_T unit, UI32_T port, BOOL_T is_present)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_SWDRV_PortSfpPresent_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_PortSfpPresent_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_PortSfpPresent_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_SWDRV_PortSfpPresent_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_SFP_PRESENT;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;
    cbdata_msg_p->is_present = is_present;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_MGR_SWDRV_PortSfpInfo
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Notify port sfp eeprom change
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h)
 *      who triggers this event
 *      unit
 *      sfp_index
 *      sfp_info_p
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_SWDRV_PortSfpInfo(UI32_T src_csc_id, UI32_T unit, UI32_T sfp_index, SWDRV_TYPE_SfpInfo_T *sfp_info_p)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_SWDRV_PortSfpInfo_CBData_T  *cbdata_msg_p;
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_PortSfpInfo_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_PortSfpInfo_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_SWDRV_PortSfpInfo_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_SFP_INFO;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->sfp_index = sfp_index;
    memcpy(&(cbdata_msg_p->sfp_info), sfp_info_p, sizeof(SWDRV_TYPE_SfpInfo_T));

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfo
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Notify port sfp DDM eeprom change
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h)
 *                          who triggers this event
 *      unit
 *      sfp_index
 *      sfp_ddm_info_p
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfo(UI32_T src_csc_id, UI32_T unit, UI32_T sfp_index, SWDRV_TYPE_SfpDdmInfo_T *sfp_ddm_info_p)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfo_CBData_T  *cbdata_msg_p;
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfo_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfo_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfo_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_SFP_DDM_INFO;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->sfp_index = sfp_index;
    memcpy(&(cbdata_msg_p->sfp_ddm_info), sfp_ddm_info_p, sizeof(SWDRV_TYPE_SfpDdmInfo_T));

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfoMeasured
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Notify port sfp DDM measured eeprom change
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h)
 *                          who triggers this event
 *      unit
 *      sfp_index
 *      sfp_ddm_info_measured_p
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfoMeasured(UI32_T src_csc_id, UI32_T unit, UI32_T sfp_index, SWDRV_TYPE_SfpDdmInfoMeasured_T *sfp_ddm_info_measured_p)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfoMeasured_CBData_T  *cbdata_msg_p;
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfoMeasured_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfoMeasured_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfoMeasured_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_SFP_DDM_INFO_MEASURED;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->sfp_index = sfp_index;
    memcpy(&(cbdata_msg_p->sfp_ddm_info_measured), sfp_ddm_info_measured_p, sizeof(SWDRV_TYPE_SfpDdmInfoMeasured_T));

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}
#endif /* #if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE) */

/*********************************
 *           IGMPSNP             *
 *********************************
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_MGR_IgmpSnoopStatusChangedCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When port move, AMTR will notify other CSC groups by this function.
 *
 * INPUT:
 *      igmpsnp_status   -- IGMP Snooping status
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_IgmpSnoopStatusChangedCallback(UI32_T src_csc_id, UI32_T igmpsnp_status)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_IgmpSnoopStatusChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_IgmpSnoopStatusChanged_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_IgmpSnoopStatusChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_IgmpSnoopStatusChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_IGMPSNP_STATUS_CHANGED;
    cbdata_msg_p->igmpsnp_status = igmpsnp_status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}

/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_MGR_IgmpSnoopRouterPortAddCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When port move, AMTR will notify other CSC groups by this function.
 *
 * INPUT:
 *      vid_ifidx        -- VLAN id
 *      lport_ifidx      -- added router port
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_IgmpSnoopRouterPortAddCallback(UI32_T src_csc_id, UI32_T vid_ifidx, UI32_T lport_ifidx)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_IgmpSnoopRouterPortChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_IgmpSnoopRouterPortChanged_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_IgmpSnoopRouterPortChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_IgmpSnoopRouterPortChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_ROUTER_PORT_ADD;
    cbdata_msg_p->vid_ifidx = vid_ifidx;
    cbdata_msg_p->lport_ifidx = lport_ifidx;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}

/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_MGR_IgmpSnoopRouterPortDeleteCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When port move, AMTR will notify other CSC groups by this function.
 *
 * INPUT:
 *      vid_ifidx        -- VLAN id
 *      lport_ifidx      -- deleted router port
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_IgmpSnoopRouterPortDeleteCallback(UI32_T src_csc_id, UI32_T vid_ifidx, UI32_T lport_ifidx)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_IgmpSnoopRouterPortChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_IgmpSnoopRouterPortChanged_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_IgmpSnoopRouterPortChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_IgmpSnoopRouterPortChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_ROUTER_PORT_DELETE;
    cbdata_msg_p->vid_ifidx = vid_ifidx;
    cbdata_msg_p->lport_ifidx = lport_ifidx;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}

/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_MGR_IgmpSnoopGroupMemberAddCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When port move, AMTR will notify other CSC groups by this function.
 *
 * INPUT:
 *      vid_ifidx        -- VLAN id
 *      mip              -- multicast ip
 *      lport_ifidx      -- added router port
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_IgmpSnoopGroupMemberAddCallback(UI32_T src_csc_id, UI32_T vid_ifidx, UI8_T *mip, UI32_T lport_ifidx)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_IgmpSnoopGroupMemberChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_IgmpSnoopGroupMemberChanged_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_IgmpSnoopGroupMemberChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_IgmpSnoopGroupMemberChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_GROUP_MEMBER_ADD;
    cbdata_msg_p->vid_ifidx = vid_ifidx;
    cbdata_msg_p->lport_ifidx = lport_ifidx;
    memcpy(cbdata_msg_p->mip, mip, sizeof(cbdata_msg_p->mip));

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}

/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_MGR_IgmpSnoopGroupMemberDeleteCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When port move, AMTR will notify other CSC groups by this function.
 *
 * INPUT:
 *      vid_ifidx        -- VLAN id
 *      mip              -- multicast ip
 *      lport_ifidx      -- added router port
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_IgmpSnoopGroupMemberDeleteCallback(UI32_T src_csc_id, UI32_T vid_ifidx, UI8_T *mip, UI32_T lport_ifidx)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_IgmpSnoopGroupMemberChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_IgmpSnoopGroupMemberChanged_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_IgmpSnoopGroupMemberChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_IgmpSnoopGroupMemberChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_GROUP_MEMBER_DELETE;
    cbdata_msg_p->vid_ifidx = vid_ifidx;
    cbdata_msg_p->lport_ifidx = lport_ifidx;
    memcpy(cbdata_msg_p->mip, mip, sizeof(cbdata_msg_p->mip));

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}

/* SYSMGMT */
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_PowerStatusChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Pwer status changed callback function
 * INPUT   :
 *              src_csc_id       -- The csc_id(SYS_MODULE_SYSMGMT defined in sys_module.h) who triggers this event
 *              unit    -- which unit
 *          power   -- which power
 *          status  --      VAL_swIndivPowerStatus_notPresent
 *                     VAL_swIndivPowerStatus_green
 *                     VAL_swIndivPowerStatus_red
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_PowerStatusChanged_CallBack(UI32_T src_csc_id,UI32_T unit, UI32_T power, UI32_T status)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_PowerStatusChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                     *syscb_msg_p;
    SYS_CALLBACK_MGR_PowerStatusChanged_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_PowerStatusChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_PowerStatusChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_POWER_STATUS_CHANGED;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->power = power;
    cbdata_msg_p->status = status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_FanStatusChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Register the callback   function, when a fan status is changed
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_SYSMGMT defined in sys_module.h) who triggers this event
 *      unit   --- which unit
 *      fan ---    which power
 *      status --- VAL_switchFanStatus_ok / VAL_switchFanStatus_failure
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_FanStatusChanged_CallBack(UI32_T src_csc_id,UI32_T unit, UI32_T fan, UI32_T status)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_FanStatusChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                     *syscb_msg_p;
    SYS_CALLBACK_MGR_FanStatusChanged_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_FanStatusChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_FanStatusChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_FAN_STATUS_CHANGED;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->fan= fan;
    cbdata_msg_p->status = status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}
#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ThermalStatusChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Send sys callback message for thermal status changed.
 *
 * INPUT:
 *      src_csc_id  -- The csc_id(SYS_MODULE_SYSMGMT defined in sys_module.h)
 *                     who triggers this event
 *      unit        -- which  unit
 *      thermal_idx -- which thermal sensor
 *      temperature -- temperature of the given thermal sensor index
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ThermalStatusChanged_CallBack(UI32_T src_csc_id,UI32_T unit, UI32_T thermal_idx, I32_T temperature)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof( SYS_CALLBACK_MGR_ThermalStatusChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                     *syscb_msg_p;
    SYS_CALLBACK_MGR_ThermalStatusChanged_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_ThermalStatusChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_ThermalStatusChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_THERMAL_STATUS_CHANGED;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->thermal_idx= thermal_idx;
    cbdata_msg_p->temperature = temperature;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}
#endif

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_XFPModuleStatusChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      XFP module   status changed callback function,
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_SYSMGMT defined in sys_module.h) who triggers this event
 *      unit   --- which  unit
 *      port ---   which port
 *      status --- insert or remove
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_XFPModuleStatusChanged_CallBack(UI32_T src_csc_id,UI32_T unit, UI32_T port, UI32_T status)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_XFPModuleStatusChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                     *syscb_msg_p;
    SYS_CALLBACK_MGR_XFPModuleStatusChanged_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_XFPModuleStatusChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_XFPModuleStatusChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_XFP_MODULE_STATUS_CHANGED;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port= port;
    cbdata_msg_p->status = status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}

/* SYSDRV */
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_SYSDRV_AlarmInputStatusChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Alarm input status changed callback function
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_SYSDRV defined in sys_module.h) who triggers this event
 *      unit             -- which unit
 *      status           -- VAL_alarmInputType_alarmInputType_1
 *                          VAL_alarmInputType_alarmInputType_2
 *                          VAL_alarmInputType_alarmInputType_3
 *                          VAL_alarmInputType_alarmInputType_4
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T  SYS_CALLBACK_MGR_SYSDRV_AlarmInputStatusChanged_Callback(UI32_T src_csc_id,UI32_T unit, UI32_T status)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AlarmInputStatusChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                     *syscb_msg_p;
    SYS_CALLBACK_MGR_AlarmInputStatusChanged_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AlarmInputStatusChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_AlarmInputStatusChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_ALARM_INPUT_STATUS_CHANGED;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->status = status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_SYSDRV_MajorAlarmOutputStatusChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Major alarm output status changed callback function
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_SYSDRV defined in sys_module.h) who triggers this event
 *      unit             -- which unit
 *      status           -- SYSDRV_ALARMMAJORSTATUS_XXX
 *                          (e.g. SYSDRV_ALARMMAJORSTATUS_POWER_STATUS_CHANGED_MASK)
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T  SYS_CALLBACK_MGR_SYSDRV_MajorAlarmOutputStatusChanged_Callback(UI32_T src_csc_id,UI32_T unit, UI32_T status)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AlarmOutputStatusChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                     *syscb_msg_p;
    SYS_CALLBACK_MGR_AlarmOutputStatusChanged_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AlarmOutputStatusChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_AlarmOutputStatusChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_MAJOR_ALARM_OUTPUT_STATUS_CHANGED;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->status = status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_SYSDRV_MinorAlarmOutputStatusChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Minor alarm output status changed callback function
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_SYSDRV defined in sys_module.h) who triggers this event
 *      unit             -- which unit
 *      status           -- SYSDRV_ALARMMINORSTATUS_XXX
 *                          (e.g. SYSDRV_ALARMMINORSTATUS_OVERHEAT_MASK)
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T  SYS_CALLBACK_MGR_SYSDRV_MinorAlarmOutputStatusChanged_Callback(UI32_T src_csc_id,UI32_T unit, UI32_T status)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AlarmOutputStatusChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                     *syscb_msg_p;
    SYS_CALLBACK_MGR_AlarmOutputStatusChanged_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AlarmOutputStatusChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_AlarmOutputStatusChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_MINOR_ALARM_OUTPUT_STATUS_CHANGED;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->status = status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_SYSDRV_PowerStatusChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Power status changed callback function
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_SYSDRV defined in sys_module.h) who triggers this event
 *              unit    -- which unit
 *          power   -- which power
 *          status  --      VAL_swIndivPowerStatus_notPresent
 *                     VAL_swIndivPowerStatus_green
 *                     VAL_swIndivPowerStatus_red
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T  SYS_CALLBACK_MGR_SYSDRV_PowerStatusChanged_Callback(UI32_T src_csc_id,UI32_T unit, UI32_T power, UI32_T status)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_PowerStatusChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                     *syscb_msg_p;
    SYS_CALLBACK_MGR_PowerStatusChanged_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_PowerStatusChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_PowerStatusChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_POWER_STATUS_CHANGED;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->power = power;
    cbdata_msg_p->status = status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

#if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_SYSDRV_PowerTypeChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Power type changed callback function
 *
 * INPUT:
 *      src_csc_id  -- The csc_id(SYS_MODULE_SYSDRV defined in sys_module.h) who triggers this event
 *      unit        -- which unit
 *      power       -- which power
 *      type        -- SYS_HWCFG_COMMON_POWER_DC_N48_MODULE_TYPE
 *                     SYS_HWCFG_COMMON_POWER_DC_P24_MODULE_TYPE
 *                     SYS_HWCFG_COMMON_POWER_AC_MODULE_TYPE
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T  SYS_CALLBACK_MGR_SYSDRV_PowerTypeChanged_CallBack(UI32_T src_csc_id,UI32_T unit, UI32_T power, UI32_T type)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_PowerTypeChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                     *syscb_msg_p;
    SYS_CALLBACK_MGR_PowerTypeChanged_CBData_T *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_PowerTypeChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_PowerTypeChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_POWER_TYPE_CHANGED;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->power= power;
    cbdata_msg_p->type = type;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}

#endif

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_SYSDRV_FanStatusChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Register the callback   function, when a fan status is changed
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_SYSDRV defined in sys_module.h) who triggers this event
 *      unit   --- which unit
 *      fan ---    which power
 *      status --- VAL_switchFanStatus_ok / VAL_switchFanStatus_failure
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_SYSDRV_FanStatusChanged_CallBack(UI32_T src_csc_id,UI32_T unit, UI32_T fan, UI32_T status)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_FanStatusChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                     *syscb_msg_p;
    SYS_CALLBACK_MGR_FanStatusChanged_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_FanStatusChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_FanStatusChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_FAN_STATUS_CHANGED;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->fan= fan;
    cbdata_msg_p->status = status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_SYSDRV_FanSpeedChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Register the callback   function, when a fan speed  is changed
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_SYSDRV defined in sys_module.h) who triggers this event
 *      unit   --- which unit
 *      fan ---    which fan
 *      speed --- fan speed
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_SYSDRV_FanSpeedChanged_CallBack(UI32_T src_csc_id,UI32_T unit, UI32_T fan, UI32_T speed)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_FanSpeedChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                     *syscb_msg_p;
    SYS_CALLBACK_MGR_FanSpeedChanged_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_FanSpeedChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_FanSpeedChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_FAN_SPEED_CHANGED;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->fan= fan;
    cbdata_msg_p->speed= speed;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}

#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_SYSDRV_ThermalStatusChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Send sys callback message for thermal status changed.
 *
 * INPUT:
 *      src_csc_id  -- The csc_id(SYS_MODULE_SYSDRV defined in sys_module.h)
 *                     who triggers this event
 *      unit        -- which unit
 *      thermal_idx -- which thermal sensor
 *      temperature -- temperature of the given thermal sensor index
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_SYSDRV_ThermalStatusChanged_CallBack(UI32_T src_csc_id,UI32_T unit, UI32_T thermal_idx, I32_T temperature)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_ThermalStatusChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                     *syscb_msg_p;
    SYS_CALLBACK_MGR_ThermalStatusChanged_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_ThermalStatusChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_ThermalStatusChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_THERMAL_STATUS_CHANGED;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->thermal_idx= thermal_idx;
    cbdata_msg_p->temperature = temperature;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}

#endif

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_SYSDRV_XFPModuleStatusChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      XFP module   status changed callback function,
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_SYSDRV defined in sys_module.h) who triggers this event
 *      unit   --- which  unit
 *      port ---  which port
 *      status --- insert or remove
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_SYSDRV_XFPModuleStatusChanged_CallBack(UI32_T src_csc_id,UI32_T unit, UI32_T port, UI32_T status)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_XFPModuleStatusChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                     *syscb_msg_p;
    SYS_CALLBACK_MGR_XFPModuleStatusChanged_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_XFPModuleStatusChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_XFPModuleStatusChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_XFP_MODULE_STATUS_CHANGED;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port= port;
    cbdata_msg_p->status = status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_SYSMGMT_XenpakStatusChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *          when Xenpak Status changed , call this function
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_SYSDRV defined in sys_module.h) who triggers this event
 *      xenpak_type    --- xenpak type
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_SYSDRV_XenpakStatusChanged_CallBack(UI32_T src_csc_id,UI32_T xenpak_type)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_XenpakStatusChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                     *syscb_msg_p;
    SYS_CALLBACK_MGR_XenpakStatusChanged_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_XenpakStatusChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_XenpakStatusChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_XENPAK_STATUS_CHANGED;
    cbdata_msg_p->xenpak_type = xenpak_type;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}

/*********************************
 *              TRK              *
 *********************************
 */
/*-----------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_AddStaticTrunkMember_CallBack
 *-----------------------------------------------------------------------------
 * PURPOSE : Call call-back function, when UIs add static trunk member.
 *
 * INPUT   : src_csc_id    -- the csc_id who triggers this event (SYS_MODULE_TRUNK)
 *           trunk_ifindex -- trunk member is added to which trunk
 *           tm_ifindex    -- which trunk member is added to trunk
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  --  Callback messages have been sent successfully
 *           FALSE --  Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_AddStaticTrunkMember_CallBack(UI32_T src_csc_id,
                                                      UI32_T trunk_ifindex,
                                                      UI32_T tm_ifindex)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AddStaticTrunkMember_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                     *syscb_msg_p;
    SYS_CALLBACK_MGR_AddStaticTrunkMember_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AddStaticTrunkMember_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_AddStaticTrunkMember_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_ADD_STATIC_TRUNK_MEMBER;
    cbdata_msg_p->trunk_ifindex = trunk_ifindex;
    cbdata_msg_p->tm_ifindex = tm_ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}

/*-----------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_DelStaticTrunkMember_CallBack
 *-----------------------------------------------------------------------------
 * PURPOSE : Call call-back function, when UIs delete static trunk member.
 *
 * INPUT   : src_csc_id    -- the csc_id who triggers this event (SYS_MODULE_TRUNK)
 *           trunk_ifindex -- trunk member is deleted from which trunk
 *           tm_ifindex    -- which trunk member is deleted from trunk
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  --  Callback messages have been sent successfully
 *           FALSE --  Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_DelStaticTrunkMember_CallBack(UI32_T src_csc_id,
                                                      UI32_T trunk_ifindex,
                                                      UI32_T tm_ifindex)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_DelStaticTrunkMember_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                     *syscb_msg_p;
    SYS_CALLBACK_MGR_DelStaticTrunkMember_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_DelStaticTrunkMember_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_DelStaticTrunkMember_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_DEL_STATIC_TRUNK_MEMBER;
    cbdata_msg_p->trunk_ifindex = trunk_ifindex;
    cbdata_msg_p->tm_ifindex = tm_ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}


/*********************************
 *              LAN              *
 *********************************
 */
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveL2muxPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When LAN received packet for L2MUX, it will notify L2MUX with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      tag_info       --- packet tag info
 *      type           --- packet type
 *      pkt_length     --- packet length
 *      src_unit       --- source unit
 *      src_port       --- source port
 *      packet_class   --- packet classified by LAN
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveL2muxPacketCallback(UI32_T         src_csc_id,
                                                   L_MM_Mref_Handle_T *mref_handle_p,
                                                   UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                   UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                   UI16_T         tag_info,
                                                   UI16_T         type,
                                                   UI32_T         pkt_length,
                                                   UI32_T         src_unit,
                                                   UI32_T         src_port,
                                                   UI32_T         packet_class)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_LanReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_L2MUX_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LanReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_L2MUX_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->tag_info           = tag_info;
    cbdata_msg_p->type               = type;
    cbdata_msg_p->pkt_length         = pkt_length;
    cbdata_msg_p->src_unit           = src_unit;
    cbdata_msg_p->src_port           = src_port;
    cbdata_msg_p->packet_class       = packet_class;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveIPPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When LAN received IP packet for L2MUX, it will notify L2MUX with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      tag_info       --- packet tag info
 *      type           --- packet type
 *      pkt_length     --- packet length
 *      src_unit       --- source unit
 *      src_port       --- source port
 *      packet_class   --- packet classified by LAN
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveIPPacketCallback(UI32_T         src_csc_id,
                                                   L_MM_Mref_Handle_T *mref_handle_p,
                                                   UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                   UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                   UI16_T         tag_info,
                                                   UI16_T         type,
                                                   UI32_T         pkt_length,
                                                   UI32_T         src_unit,
                                                   UI32_T         src_port,
                                                   UI32_T         packet_class)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_LanReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_IP_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LanReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_IP_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->tag_info           = tag_info;
    cbdata_msg_p->type               = type;
    cbdata_msg_p->pkt_length         = pkt_length;
    cbdata_msg_p->src_unit           = src_unit;
    cbdata_msg_p->src_port           = src_port;
    cbdata_msg_p->packet_class       = packet_class;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveLacpPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When LAN received packet for LACP, it will notify LACP with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      tag_info       --- packet tag info
 *      type           --- packet type
 *      pkt_length     --- packet length
 *      src_unit       --- source unit
 *      src_port       --- source port
 *      packet_class   --- packet classified by LAN
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveLacpPacketCallback(UI32_T         src_csc_id,
                                                  L_MM_Mref_Handle_T *mref_handle_p,
                                                  UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                  UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                  UI16_T         tag_info,
                                                  UI16_T         type,
                                                  UI32_T         pkt_length,
                                                  UI32_T         src_unit,
                                                  UI32_T         src_port,
                                                  UI32_T         packet_class)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_LanReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LACP_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LanReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LACP_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->tag_info           = tag_info;
    cbdata_msg_p->type               = type;
    cbdata_msg_p->pkt_length         = pkt_length;
    cbdata_msg_p->src_unit           = src_unit;
    cbdata_msg_p->src_port           = src_port;
    cbdata_msg_p->packet_class       = packet_class;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveOamPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When LAN received packet for OAM, it will notify OAM with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      tag_info       --- packet tag info
 *      type           --- packet type
 *      pkt_length     --- packet length
 *      src_unit       --- source unit
 *      src_port       --- source port
 *      packet_class   --- packet classified by LAN
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveOamPacketCallback(UI32_T         src_csc_id,
                                                 L_MM_Mref_Handle_T *mref_handle_p,
                                                 UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                 UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                 UI16_T         tag_info,
                                                 UI16_T         type,
                                                 UI32_T         pkt_length,
                                                 UI32_T         src_unit,
                                                 UI32_T         src_port,
                                                 UI32_T         packet_class)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_LanReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_OAM_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LanReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_OAM_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->tag_info           = tag_info;
    cbdata_msg_p->type               = type;
    cbdata_msg_p->pkt_length         = pkt_length;
    cbdata_msg_p->src_unit           = src_unit;
    cbdata_msg_p->src_port           = src_port;
    cbdata_msg_p->packet_class       = packet_class;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveOamLbkPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When LAN received packet for OAM Loopback, it will notify OAM with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      tag_info       --- packet tag info
 *      type           --- packet type
 *      pkt_length     --- packet length
 *      src_unit       --- source unit
 *      src_port       --- source port
 *      packet_class   --- packet classified by LAN
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveOamLbkPacketCallback(UI32_T         src_csc_id,
                                                 L_MM_Mref_Handle_T *mref_handle_p,
                                                 UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                 UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                 UI16_T         tag_info,
                                                 UI16_T         type,
                                                 UI32_T         pkt_length,
                                                 UI32_T         src_unit,
                                                 UI32_T         src_port,
                                                 UI32_T         packet_class)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_LanReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_OAM_LBK_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LanReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_OAM_LBK_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->tag_info           = tag_info;
    cbdata_msg_p->type               = type;
    cbdata_msg_p->pkt_length         = pkt_length;
    cbdata_msg_p->src_unit           = src_unit;
    cbdata_msg_p->src_port           = src_port;
    cbdata_msg_p->packet_class       = packet_class;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveDot1xPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When LAN received packet for DOT1X, it will notify DOT1X with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      tag_info       --- packet tag info
 *      type           --- packet type
 *      pkt_length     --- packet length
 *      src_unit       --- source unit
 *      src_port       --- source port
 *      packet_class   --- packet classified by LAN
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveDot1xPacketCallback(UI32_T         src_csc_id,
                                                  L_MM_Mref_Handle_T *mref_handle_p,
                                                  UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                  UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                  UI16_T         tag_info,
                                                  UI16_T         type,
                                                  UI32_T         pkt_length,
                                                  UI32_T         src_unit,
                                                  UI32_T         src_port,
                                                  UI32_T         packet_class)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_LanReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_DOT1X_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LanReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_DOT1X_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->tag_info           = tag_info;
    cbdata_msg_p->type               = type;
    cbdata_msg_p->pkt_length         = pkt_length;
    cbdata_msg_p->src_unit           = src_unit;
    cbdata_msg_p->src_port           = src_port;
    cbdata_msg_p->packet_class       = packet_class;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveSflowPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When LAN received packet for SFLOW, it will notify SFLOW with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      tag_info       --- packet tag info
 *      type           --- packet type
 *      pkt_length     --- packet length
 *      src_unit       --- source unit
 *      src_port       --- source port
 *      packet_class   --- packet classified by LAN
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveSflowPacketCallback(UI32_T         src_csc_id,
                                                  L_MM_Mref_Handle_T *mref_handle_p,
                                                  UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                  UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                  UI16_T         tag_info,
                                                  UI16_T         type,
                                                  UI32_T         pkt_length,
                                                  UI32_T         src_unit,
                                                  UI32_T         src_port,
                                                  UI32_T         packet_class)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_LanReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_SFLOW_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LanReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_SFLOW_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->tag_info           = tag_info;
    cbdata_msg_p->type               = type;
    cbdata_msg_p->pkt_length         = pkt_length;
    cbdata_msg_p->src_unit           = src_unit;
    cbdata_msg_p->src_port           = src_port;
    cbdata_msg_p->packet_class       = packet_class;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveLoopbackPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When LAN received internal loopback packet, it will notify swctrl with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      tag_info       --- packet tag info
 *      type           --- packet type
 *      pkt_length     --- packet length
 *      src_unit       --- source unit
 *      src_port       --- source port
 *      packet_class   --- packet classified by LAN
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveLoopbackPacketCallback(UI32_T         src_csc_id,
                                                  L_MM_Mref_Handle_T *mref_handle_p,
                                                  UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                  UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                  UI16_T         tag_info,
                                                  UI16_T         type,
                                                  UI32_T         pkt_length,
                                                  UI32_T         src_unit,
                                                  UI32_T         src_port,
                                                  UI32_T         packet_class)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_LanReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LOOPBACK_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LanReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LOOPBACK_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->tag_info           = tag_info;
    cbdata_msg_p->type               = type;
    cbdata_msg_p->pkt_length         = pkt_length;
    cbdata_msg_p->src_unit           = src_unit;
    cbdata_msg_p->src_port           = src_port;
    cbdata_msg_p->packet_class       = packet_class;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveLbdPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When LAN received packet for LBD, it will notify LBD with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      tag_info       --- packet tag info
 *      type           --- packet type
 *      pkt_length     --- packet length
 *      src_unit       --- source unit
 *      src_port       --- source port
 *      packet_class   --- packet classified by LAN
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveLbdPacketCallback(UI32_T             src_csc_id,
                                                 L_MM_Mref_Handle_T *mref_handle_p,
                                                 UI8_T              dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                 UI8_T              src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                 UI16_T             tag_info,
                                                 UI16_T             type,
                                                 UI32_T             pkt_length,
                                                 UI32_T             src_unit,
                                                 UI32_T             src_port,
                                                 UI32_T             packet_class)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_LanReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LBD_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LanReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LBD_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->tag_info           = tag_info;
    cbdata_msg_p->type               = type;
    cbdata_msg_p->pkt_length         = pkt_length;
    cbdata_msg_p->src_unit           = src_unit;
    cbdata_msg_p->src_port           = src_port;
    cbdata_msg_p->packet_class       = packet_class;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_AuthenticatePacket
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When LAN received packet, it will notify upper layer to authenticate
 *      it.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      tag_info       --- packet tag info
 *      type           --- packet type
 *      pkt_length     --- packet length
 *      src_unit       --- source unit
 *      src_port       --- source port
 *      packet_class   --- packet classified by LAN
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_AuthenticatePacket(
    UI32_T         src_csc_id,
    L_MM_Mref_Handle_T *mref_handle_p,
    UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI16_T         tag_info,
    UI16_T         type,
    UI32_T         pkt_length,
    UI32_T         src_unit,
    UI32_T         src_port,
    UI32_T         packet_class)
{
    SYS_CALLBACK_MGR_AuthenticatePacket_CBData_T    cbdata_msg;
    SYS_CALLBACK_MGR_LanReceivePacket_CBData_T      *lan_cbdata_msg_p;

    lan_cbdata_msg_p = &cbdata_msg.lan_cbdata;

    cbdata_msg.auth_result = SYS_CALLBACK_MGR_AUTH_BYPASS;
    cbdata_msg.flag = 0;
    lan_cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    lan_cbdata_msg_p->tag_info           = tag_info;
    lan_cbdata_msg_p->type               = type;
    lan_cbdata_msg_p->pkt_length         = pkt_length;
    lan_cbdata_msg_p->src_unit           = src_unit;
    lan_cbdata_msg_p->src_port           = src_port;
    lan_cbdata_msg_p->packet_class       = packet_class;
    memcpy(lan_cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(lan_cbdata_msg_p->dst_mac, dst_mac, 6);

    return SYS_CALLBACK_MGR_AuthenticatePacket_AsyncReturn(src_csc_id, SYS_CALLBACK_MGR_AUTH_BYPASS, &cbdata_msg);
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_AuthenticatePacket_AsyncReturn
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      This function will check if packet pass the authentication in different CSC.
 *      After checking, CSC will notify next CSC for further process.
 *
 * INPUT:
 *      src_csc_id  --   The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      result      --   authentication result
 *      cookie      --   callback function argument
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_AuthenticatePacket_AsyncReturn(
    UI32_T src_csc_id,
    SYS_CALLBACK_MGR_AuthenticatePacket_Result_T result,
    void *cookie)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AuthenticatePacket_CBData_T)))];
    SYSFUN_Msg_T                                    *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                          *syscb_msg_p;
    SYS_CALLBACK_MGR_AuthenticatePacket_CBData_T    *cbdata_msg_p;
    SYS_CALLBACK_MGR_AuthenticatePacket_CBData_T    *org_cbdata_msg_p = cookie;
    L_MM_Mref_Handle_T *mref_handle_p;
    BOOL_T sent_to_next_csc = TRUE;
    BOOL_T free_org_cbdata_msg = TRUE;
    BOOL_T ret = TRUE;

    mref_handle_p = L_IPCMEM_GetPtr(org_cbdata_msg_p->lan_cbdata.mref_handle_offset);

    if (result == SYS_CALLBACK_MGR_AUTH_FAILED)
    {
        sent_to_next_csc = FALSE;
    }

    /* flag == 0 means it is called by SYS_CALLBACK_MGR_AuthenticatePacket,
     * org_cbdata_msg_p is allocated by stack (not by malloc),
     * so no need to free.
     */
    if(org_cbdata_msg_p->flag == 0)
    {
        free_org_cbdata_msg = FALSE;
    }

    if (sent_to_next_csc)
    {
        sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AuthenticatePacket_CBData_T));
        syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
        cbdata_msg_p= (SYS_CALLBACK_MGR_AuthenticatePacket_CBData_T*)syscb_msg_p->callback_data;

        syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_AUTHENTICATE_PACKET;
        memcpy(cbdata_msg_p, org_cbdata_msg_p, sizeof(*cbdata_msg_p));

        if (cbdata_msg_p->auth_result < result)
        {
            cbdata_msg_p->auth_result = result;
        }

        if (!SYS_CALLBACK_MGR_SendMsgWithMsgQKey(
                src_csc_id,
                lan_receive_authenticate_packet_msgqkey_list[0],
                sysfun_msg_p,
                FALSE))
        {
            L_MM_Mref_Release(&mref_handle_p);
            ret = FALSE;
        }
    }
    else
    {
        L_MM_Mref_Release(&mref_handle_p);
    }

    if (free_org_cbdata_msg)
    {
        L_MM_Free(org_cbdata_msg_p);
    }

    return ret;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_AuthenticationDispatchPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:  After sys_callback authentication process is done, dispatch packet to lan
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- MREF handle for packet
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      tag_info       --- packet tag info
 *      type           --- packet type
 *      pkt_length     --- packet length
 *      src_unit       --- source unit
 *      src_port       --- source port
 *      packet_class   --- packet classified by LAN
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_AuthenticationDispatchPacketCallback(
    UI32_T             src_csc_id,
    L_MM_Mref_Handle_T *mref_handle_p,
    UI8_T              dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T              src_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI16_T             tag_info,
    UI16_T             type,
    UI32_T             pkt_length,
    UI32_T             src_unit,
    UI32_T             src_port,
    UI32_T             packet_class)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                 *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                       *syscb_msg_p;
    SYS_CALLBACK_MGR_LanReceivePacket_CBData_T   *cbdata_msg_p;
    UI32_T                                       fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_AUTHENTICATION_DISPATCH);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LanReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_AUTHENTICATION_DISPATCH;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->pkt_length         = pkt_length;
    cbdata_msg_p->tag_info           = tag_info;
    cbdata_msg_p->type               = type;
    cbdata_msg_p->src_unit           = src_unit;
    cbdata_msg_p->src_port           = src_port;
    cbdata_msg_p->packet_class       = packet_class;
    memcpy(cbdata_msg_p->src_mac, src_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, SYS_ADPT_MAC_ADDR_LEN);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_RxSnoopDhcpPacket
 *-------------------------------------------------------------------------
 * PURPOSE:  When L2MUX receive dhcp packet, it will notify corresponding
 *           snooping protocol to process it
 * INPUT  :  src_csc_id      -- The csc_id(SYS_MODULE_L2MUX defined in sys_module.h) who triggers this event
 *           mref_handle_p   -- The memory reference address
 *           dst_mac         -- destination mac address
 *           src_mac         -- source mac address
 *           tag_info        -- tag information
 *           ether_type      -- ethernet type
 *           pkt_length      -- packet length
 *           lport           -- receive logical port
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_RxSnoopDhcpPacket(
    UI32_T   src_csc_id,
    L_MM_Mref_Handle_T *mref_handle_p,
    UI8_T    dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T    src_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI16_T   tag_info,
    UI16_T   ether_type,
    UI32_T   pkt_length,
    UI32_T   lport)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                 *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                       *syscb_msg_p;
    SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T *cbdata_msg_p;
    UI32_T                                       fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RX_SNOOP_DHCP_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RX_SNOOP_DHCP_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->pkt_length         = pkt_length;
    cbdata_msg_p->tag_info           = tag_info;
    cbdata_msg_p->type               = ether_type;
    cbdata_msg_p->lport              = lport;
    memcpy(cbdata_msg_p->src_mac, src_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, SYS_ADPT_MAC_ADDR_LEN);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_RxDhcpPacket
 *-------------------------------------------------------------------------
 * PURPOSE:  After SYS_CALLBACK_GROUP process dhcp packet,
 *           it will notify L2MUX to continue the receving packet flow
 * INPUT  :  src_csc_id      -- The csc_id(SYS_MODULE_L2MUX defined in sys_module.h) who triggers this event
 *           mref_handle_p   -- The memory reference address
 *           dst_mac         -- destination mac address
 *           src_mac         -- source mac address
 *           tag_info        -- tag information
 *           ether_type      -- ethernet type
 *           pkt_length      -- packet length
 *           lport           -- receive logical port
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_RxDhcpPacket(
    UI32_T   src_csc_id,
    L_MM_Mref_Handle_T *mref_handle_p,
    UI8_T               dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T               src_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI16_T              tag_info,
    UI16_T              ether_type,
    UI32_T              pkt_len,
    UI32_T              ing_lport)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                    *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                          *syscb_msg_p;
    SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T *cbdata_msg_p;
    UI32_T                                       fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RX_DHCP_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RX_DHCP_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->pkt_length         = pkt_len;
    cbdata_msg_p->tag_info           = tag_info;
    cbdata_msg_p->type               = ether_type;
    cbdata_msg_p->lport              = ing_lport;
    memcpy(cbdata_msg_p->src_mac, src_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, SYS_ADPT_MAC_ADDR_LEN);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_TxSnoopDhcpPacket
 *-------------------------------------------------------------------------
 * PURPOSE:  When IML send dhcp packet, it will notify corresponding
 *           snooping protocol to process it
 * INPUT  :
 *           src_csc_id      -- The csc_id(SYS_MODULE_IML defined in sys_module.h) who triggers this event
 *           mref_handle_p   -- The memory reference address
 *           pkt_len         -- packet length
 *           dst_mac         -- destination mac address
 *           src_mac         -- source mac address
 *           egr_vidifindex  -- egress vlan id ifindex
 *           egr_lport       -- egress lport(if specified, only send to this port)
 *           ing_lport       -- ingress lport (0 means sent by DUT)
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_TxSnoopDhcpPacket(
    UI32_T   src_csc_id,
    L_MM_Mref_Handle_T *mref_handle_p,
    UI32_T   pkt_len,
    UI8_T    dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T    src_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI32_T   egr_vidifindex,
    UI32_T   egr_lport,
    UI32_T   ing_lport)
{
#if(SYS_CPNT_DHCPSNP == TRUE)
   {
        UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_TxSnoopDhcpPacket_CBData_T)))];
        SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
        SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
        SYS_CALLBACK_MGR_TxSnoopDhcpPacket_CBData_T *cbdata_msg_p;
        UI32_T                                      fail_count = 0;

        SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_TX_SNOOP_DHCP_PACKET);

        sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_TxSnoopDhcpPacket_CBData_T));
        syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
        cbdata_msg_p= (SYS_CALLBACK_MGR_TxSnoopDhcpPacket_CBData_T*)syscb_msg_p->callback_data;

        syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_TX_SNOOP_DHCP_PACKET;
        cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
        cbdata_msg_p->egr_vidifindex     = egr_vidifindex;
        cbdata_msg_p->egr_lport          = egr_lport;
        cbdata_msg_p->ing_lport          = ing_lport;
        cbdata_msg_p->pkt_len            = pkt_len;
        memcpy(cbdata_msg_p->dst_mac, dst_mac, SYS_ADPT_MAC_ADDR_LEN);
        memcpy(cbdata_msg_p->src_mac, src_mac, SYS_ADPT_MAC_ADDR_LEN);

        if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
        {
            while (fail_count>0)
            {
                fail_count--;
                L_MM_Mref_Release(&mref_handle_p);
            };
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }
#endif
    return FALSE;
}





/*********************************
 *              ISC              *
 *********************************
 */
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveStkTplgPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When ISC received packet for STKTPLG, it will notify STKTPLG with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      key_p          --- ISC key pointer
 *      mref_handle_p  --- The memory reference address
 *      rx_port        --- receive port from chip
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveStktplgPacketCallback(UI32_T         src_csc_id,
                                                     ISC_Key_T      *isc_key_p,
                                                     L_MM_Mref_Handle_T *mref_handle_p,
                                                     UI32_T         rx_port)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_IscReceiveStktplgPacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_IscReceiveStktplgPacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_STKTPLG_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_IscReceiveStktplgPacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_IscReceiveStktplgPacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_STKTPLG_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->rx_port            = rx_port;
    memcpy(&(cbdata_msg_p->isc_key), isc_key_p, sizeof(ISC_Key_T));

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*********************************
 *              L2MUX            *
 *********************************
 */
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveStaPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When LAN received packet for STA, it will notify STA with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      tag_info       --- packet tag info
 *      type           --- packet type
 *      pkt_length     --- packet length
 *      lport          --- source lport id
 *      src_unit       --- source unit
 *      src_port       --- source port
 *      packet_class   --- packet classified by LAN
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveStaPacketCallback(UI32_T         src_csc_id,
                                                 L_MM_Mref_Handle_T *mref_handle_p,
                                                 UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                 UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                 UI16_T         tag_info,
                                                 UI16_T         packet_type,
                                                 UI32_T         pkt_length,
                                                 UI32_T         src_unit,
                                                 UI32_T         src_port,
                                                 UI32_T         packet_class)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_LanReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_STA_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LanReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_STA_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->tag_info           = tag_info;
    cbdata_msg_p->type               = packet_type;
    cbdata_msg_p->pkt_length         = pkt_length;
    cbdata_msg_p->src_unit = src_unit;
    cbdata_msg_p->src_port = src_port;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
#if 0
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveGvrpPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When LAN received packet for GVRP, it will notify GVRP with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      tag_info       --- packet tag info
 *      type           --- packet type
 *      pkt_length     --- packet length
 *      lport          --- source lport id
 *      packet_class   --- packet classified by LAN
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveGvrpPacketCallback(UI32_T         src_csc_id,
                                                  L_MM_Mref_Handle_T *mref_handle_p,
                                                  UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                  UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                  UI16_T         tag_info,
                                                  UI16_T         type,
                                                  UI32_T         pkt_length,
                                                  UI32_T         lport)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_GVRP_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_GVRP_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->tag_info           = tag_info;
    cbdata_msg_p->type               = type;
    cbdata_msg_p->pkt_length         = pkt_length;
    cbdata_msg_p->lport              = lport;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveLldpPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When LAN received packet for LLDP, it will notify LLDP with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      tag_info       --- packet tag info
 *      type           --- packet type
 *      pkt_length     --- packet length
 *      lport          --- source lport id
 *      packet_class   --- packet classified by LAN
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveLldpPacketCallback(UI32_T         src_csc_id,
                                                  L_MM_Mref_Handle_T *mref_handle_p,
                                                  UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                  UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                  UI16_T         tag_info,
                                                  UI16_T         type,
                                                  UI32_T         pkt_length,
                                                  UI32_T         lport)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LLDP_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LLDP_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->tag_info           = tag_info;
    cbdata_msg_p->type               = type;
    cbdata_msg_p->pkt_length         = pkt_length;
    cbdata_msg_p->lport              = lport;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
#else
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveGvrpPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When LAN received packet for GVRP, it will notify GVRP with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      tag_info       --- packet tag info
 *      type           --- packet type
 *      pkt_length     --- packet length
 *      src_unit          --- source unit
 *      src_port        -----source port
 *      packet_class   --- packet classified by LAN
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveGvrpPacketCallback(UI32_T         src_csc_id,
                                                 L_MM_Mref_Handle_T *mref_handle_p,
                                                 UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                 UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                 UI16_T         tag_info,
                                                 UI16_T         packet_type,
                                                 UI32_T         pkt_length,
                                                 UI32_T         src_unit,
                                                 UI32_T         src_port,
                                                 UI32_T         packet_class)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_LanReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_GVRP_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LanReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_GVRP_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->tag_info           = tag_info;
    cbdata_msg_p->type               = packet_type;
    cbdata_msg_p->pkt_length         = pkt_length;
    cbdata_msg_p->src_unit = src_unit;
    cbdata_msg_p->src_port = src_port;
    cbdata_msg_p->packet_class = packet_class;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveLldpPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When LAN received packet for LLDP, it will notify LLDP with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      tag_info       --- packet tag info
 *      type           --- packet type
 *      pkt_length     --- packet length
 *      src_unit          --- source unit
 *      src_port        -----source port
 *      packet_class   --- packet classified by LAN
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveLldpPacketCallback(UI32_T         src_csc_id,
                                                 L_MM_Mref_Handle_T *mref_handle_p,
                                                 UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                 UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                 UI16_T         tag_info,
                                                 UI16_T         packet_type,
                                                 UI32_T         pkt_length,
                                                 UI32_T         src_unit,
                                                 UI32_T         src_port,
                                                 UI32_T         packet_class)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_LanReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LLDP_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LanReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LLDP_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->tag_info           = tag_info;
    cbdata_msg_p->type               = packet_type;
    cbdata_msg_p->pkt_length         = pkt_length;
    cbdata_msg_p->src_unit = src_unit;
    cbdata_msg_p->src_port = src_port;
    cbdata_msg_p->packet_class = packet_class;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

#endif

//#if (SYS_CPNT_SYNCE == TRUE)
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveESMCPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When LAN received packet for SyncE SSM, it will notify LLDP with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      tag_info       --- packet tag info
 *      type           --- packet type
 *      pkt_length     --- packet length
 *      src_unit          --- source unit
 *      src_port        -----source port
 *      packet_class   --- packet classified by LAN
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveESMCPacketCallback(UI32_T         src_csc_id,
                                                 L_MM_Mref_Handle_T *mref_handle_p,
                                                 UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                 UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                 UI16_T         tag_info,
                                                 UI16_T         packet_type,
                                                 UI32_T         pkt_length,
                                                 UI32_T         src_unit,
                                                 UI32_T         src_port,
                                                 UI32_T         packet_class)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_LanReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_ESMC_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LanReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_ESMC_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->tag_info           = tag_info;
    cbdata_msg_p->type               = packet_type;
    cbdata_msg_p->pkt_length         = pkt_length;
    cbdata_msg_p->src_unit = src_unit;
    cbdata_msg_p->src_port = src_port;
    cbdata_msg_p->packet_class = packet_class;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
//#endif

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveIgmpsnpPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When LAN received packet for IGMPSNP, it will notify IGMPSNP with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      tag_info       --- packet tag info
 *      type           --- packet type
 *      pkt_length     --- packet length
 *      lport          --- source lport id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveIgmpsnpPacketCallback(UI32_T         src_csc_id,
                                                     L_MM_Mref_Handle_T *mref_handle_p,
                                                     UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                     UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                     UI16_T         tag_info,
                                                     UI16_T         type,
                                                     UI32_T         pkt_length,
                                                     UI32_T         lport)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_IGMPSNP_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_IGMPSNP_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->tag_info           = tag_info;
    cbdata_msg_p->type               = type;
    cbdata_msg_p->pkt_length         = pkt_length;
    cbdata_msg_p->lport              = lport;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveMldsnpPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When LAN received packet for mldsnp, it will notify mldsnp with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      tag_info       --- packet tag info
 *      type           --- packet type
 *      pkt_length     --- packet length
 *      ext_hdr_len    --- ipv6 extension header length
 *      lport          --- source lport id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveMldsnpPacketCallback(UI32_T         src_csc_id,
                                                     L_MM_Mref_Handle_T *mref_handle_p,
                                                     UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                     UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                     UI16_T         tag_info,
                                                     UI16_T         type,
                                                     UI32_T         pkt_length,
                                                     UI32_T         ext_hdr_len,
                                                     UI32_T         lport)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_MldsnpReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_MldsnpReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_MLDSNP_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_MldsnpReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_MldsnpReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_MLDSNP_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->tag_info           = tag_info;
    cbdata_msg_p->type               = type;
    cbdata_msg_p->pkt_length         = pkt_length;
    cbdata_msg_p->lport              = lport;
    cbdata_msg_p->ext_hdr_len        = ext_hdr_len;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


/*********************************
 *              IML              *
 *********************************
 */
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveBootpPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When IML received packet for BOOTP, it will notify IP service with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_IML defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      packet_length  --- the length of the receive packet
 *      ifindex        --- the interface index where the packet is received
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      ingress_vid    --- ingress vlan id
 *      src_port       --- source lport id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveBootpPacketCallback(UI32_T   src_csc_id,
                                                   L_MM_Mref_Handle_T *mref_handle_p,
                                                   UI32_T   packet_length,
                                                   UI32_T   ifindex,
                                                   UI8_T    dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                   UI8_T    src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                   UI16_T   ingress_vid,
                                                   UI32_T   src_port)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_ImlReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_ImlReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_BOOTP_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_ImlReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_ImlReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_BOOTP_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->packet_length      = packet_length;
    cbdata_msg_p->ifindex            = ifindex;
    cbdata_msg_p->ingress_vid        = ingress_vid;
    cbdata_msg_p->src_port           = src_port;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveUdpHelperPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When IML received packet for UDPHELPER, it will notify IP service with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      packet_length  --- the length of the receive packet
 *      ifindex        --- the interface index where the packet is received
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      ingress_vid    --- ingress vlan id
 *      src_port       --- source lport id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveUdpHelperPacketCallback(UI32_T   src_csc_id,
                                                       L_MM_Mref_Handle_T *mref_handle_p,
                                                       UI32_T   packet_length,
                                                       UI32_T   ifindex,
                                                       UI8_T    dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                       UI8_T    src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                       UI16_T   ingress_vid,
                                                       UI32_T   src_port)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_ImlReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_ImlReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_UDP_HELPER_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_ImlReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_ImlReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_UDP_HELPER_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->packet_length      = packet_length;
    cbdata_msg_p->ifindex            = ifindex;
    cbdata_msg_p->ingress_vid        = ingress_vid;
    cbdata_msg_p->src_port           = src_port;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveArpPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When IML received packet for ARP, it will notify IP service with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      packet_length  --- the length of the receive packet
 *      ifindex        --- the interface index where the packet is received
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      ingress_vid    --- ingress vlan id
 *      src_port       --- source lport id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveArpPacketCallback(UI32_T   src_csc_id,
                                                                   L_MM_Mref_Handle_T *mref_handle_p,
                                                                   UI32_T   packet_length,
                                                                   UI32_T   ifindex,
                                                                   UI8_T    dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                                   UI8_T    src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                                   UI16_T   ingress_vid,
                                                                   UI32_T   src_port)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_ImlReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_ImlReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_ARP_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_ImlReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_ImlReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_ARP_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->packet_length      = packet_length;
    cbdata_msg_p->ifindex            = ifindex;
    cbdata_msg_p->ingress_vid        = ingress_vid;
    cbdata_msg_p->src_port           = src_port;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveHsrpPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When IML received packet for HSRP, it will notify IP service with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      packet_length  --- the length of the receive packet
 *      ifindex        --- the interface index where the packet is received
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      ingress_vid    --- ingress vlan id
 *      src_port       --- source lport id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveHsrpPacketCallback(UI32_T   src_csc_id,
                                                  L_MM_Mref_Handle_T *mref_handle_p,
                                                  UI32_T   packet_length,
                                                  UI32_T   ifindex,
                                                  UI8_T    dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                  UI8_T    src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                  UI16_T   ingress_vid,
                                                  UI32_T   src_port)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_ImlReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_ImlReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_HSRP_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_ImlReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_ImlReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_HSRP_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->packet_length      = packet_length;
    cbdata_msg_p->ifindex            = ifindex;
    cbdata_msg_p->ingress_vid        = ingress_vid;
    cbdata_msg_p->src_port           = src_port;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveVrrpPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When IML received packet for VRRP, it will notify IP service with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      packet_length  --- the length of the receive packet
 *      ifindex        --- the interface index where the packet is received
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      ingress_vid    --- ingress vlan id
 *      src_port       --- source lport id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveVrrpPacketCallback(UI32_T   src_csc_id,
                                                  L_MM_Mref_Handle_T *mref_handle_p,
                                                  UI32_T   packet_length,
                                                  UI32_T   ifindex,
                                                  UI8_T    dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                  UI8_T    src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                  UI16_T   ingress_vid,
                                                  UI32_T   src_port)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_ImlReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_ImlReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_VRRP_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_ImlReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_ImlReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_VRRP_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->packet_length      = packet_length;
    cbdata_msg_p->ifindex            = ifindex;
    cbdata_msg_p->ingress_vid        = ingress_vid;
    cbdata_msg_p->src_port           = src_port;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

#if (SYS_CPNT_DHCPV6SNP == TRUE)
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveDhcpv6snpPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When IML received packet for DHCPV6SNP, it will notify dhcpv6snp with this function.
 *
 * INPUT:
 *      src_csc_id         --- The csc_id(SYS_MODULE_IML defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      packet_length  --- the length of the receive packet
 *      ext_hdr_len    --- ipv6 extension header length
 *      ifindex            --- the interface index where the packet is received
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      ingress_vid    --- ingress vlan id
 *      src_port       --- source lport id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveDhcpv6snpPacketCallback(UI32_T   src_csc_id,
                                                   L_MM_Mref_Handle_T *mref_handle_p,
                                                   UI32_T   packet_length,
                                                   UI32_T   ext_hdr_len,
                                                   UI8_T    dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                   UI8_T    src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                   UI16_T   ingress_vid,
                                                   UI32_T   src_port)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_Dhcp6snpReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T             *syscb_msg_p;
    SYS_CALLBACK_MGR_Dhcp6snpReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_DHCPV6SNP_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_Dhcp6snpReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_Dhcp6snpReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_DHCPV6SNP_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->packet_length      = packet_length;
    cbdata_msg_p->ext_hdr_len        = ext_hdr_len;
    cbdata_msg_p->ingress_vid        = ingress_vid;
    cbdata_msg_p->src_port           = src_port;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
#endif /* #if (SYS_CPNT_DHCPV6SNP == TRUE) */


/*********************************
 *              STKTPLG          *
 *********************************
 */
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_StackStateCallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When stack state change, STKTPLG will notify this event.
 *
 * INPUT:
 *      src_csc_id -- The csc_id(SYS_MODULE_STKTPLG defined in sys_module.h) who
 *                    triggers this event
 *      msg        -- message used to notify the event.
 *                    STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE
 *                    STKTPLG_MASTER_WIN_AND_ENTER_MASTER_MODE
 *                    STKTPLG_MASTER_LOSE_MSG
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_StackStateCallBack(UI32_T src_csc_id,UI32_T msg)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_StackState_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                     *syscb_msg_p;
    SYS_CALLBACK_MGR_StackState_CBData_T       *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_StackState_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_StackState_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_STACK_STATE;
    cbdata_msg_p->msg = msg;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ModuleStateChangedCallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When module state changes, STKTPLG will notify this event.
 *
 * INPUT:
 *      src_csc_id -- The csc_id(SYS_MODULE_STKTPLG defined in sys_module.h) who
 *                    triggers this event
 *      unit_id    -- The module of which module state is changed.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ModuleStateChangedCallBack(UI32_T src_csc_id,UI32_T unit_id)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_ModuleStateChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                       *syscb_msg_p;
    SYS_CALLBACK_MGR_ModuleStateChanged_CBData_T *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_ModuleStateChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_ModuleStateChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_MODULE_STATE_CHANGED;
    cbdata_msg_p->unit_id = unit_id;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}

#if (SYS_CPNT_CFM==TRUE)
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveCfmPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When LAN received packet for cfm, it will notify cfm with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      tag_info       --- packet tag info
 *      type           --- packet type
 *      pkt_length     --- packet length
 *      lport          --- source lport id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveCfmPacketCallback(UI32_T         src_csc_id,
                                                     L_MM_Mref_Handle_T *mref_handle_p,
                                                     UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                     UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                     UI16_T         tag_info,
                                                     UI16_T         type,
                                                     UI32_T         pkt_length,
                                                     UI32_T         lport)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_CFM_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_CFM_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->tag_info           = tag_info;
    cbdata_msg_p->type               = type;
    cbdata_msg_p->pkt_length         = pkt_length;
    cbdata_msg_p->lport              = lport;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
#endif /* #if (SYS_CPNT_CFM==TRUE) */

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_UnitHotInsertRemoveCallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When add or remove a unit,which is slave, STKTPLG will notify this event.
 *
 * INPUT:
 *      src_csc_id -- The csc_id(SYS_MODULE_STKTPLG defined in sys_module.h) who
 *                    triggers this event
 *      msg        -- message used to notify the event.
 *                    STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE
 *                    STKTPLG_MASTER_WIN_AND_ENTER_MASTER_MODE
 *                    STKTPLG_MASTER_LOSE_MSG
 *                    STKTPLG_UNIT_HOT_INSERT_REMOVE
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_UnitHotInsertRemoveCallBack (UI32_T src_csc_id,UI32_T msg)
{
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(0))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(0);
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_UnitHotInsertRemove;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}
#endif
/*********************************
 *              STKCTRL          *
 *********************************
 */
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_SavingConfigStatusCallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Callback to UI to notify the saving config status.
 *
 * INPUT:
 *      src_csc_id -- The csc_id(SYS_MODULE_STKCTRL defined in sys_module.h) who
 *                    triggers this event
 *      status     -- state used to notify registed functions,
 *                    "TRUE" state means that saving operation is complete.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_SavingConfigStatusCallBack(UI32_T src_csc_id, UI32_T status)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SavingConfigStatus_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                       *syscb_msg_p;
    SYS_CALLBACK_MGR_SavingConfigStatus_CBData_T *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_SavingConfigStatus_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_SavingConfigStatus_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SAVING_CONFIG_STATUS;
    cbdata_msg_p->status = status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveUdldPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When LAN received packet for UDLD, it will notify UDLD with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      tag_info       --- packet tag info
 *      type           --- packet type
 *      pkt_length     --- packet length
 *      src_unit       --- source unit
 *      src_port       --- source port
 *      packet_class   --- packet classified by LAN
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveUdldPacketCallback(
    UI32_T              src_csc_id,
    L_MM_Mref_Handle_T  *mref_handle_p,
    UI8_T               dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T               src_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI16_T              tag_info,
    UI16_T              type,
    UI32_T              pkt_length,
    UI32_T              src_unit,
    UI32_T              src_port,
    UI32_T              packet_class)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_LanReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_UDLD_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LanReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_UDLD_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->tag_info           = tag_info;
    cbdata_msg_p->type               = type;
    cbdata_msg_p->pkt_length         = pkt_length;
    cbdata_msg_p->src_unit           = src_unit;
    cbdata_msg_p->src_port           = src_port;
    cbdata_msg_p->packet_class       = packet_class;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*********************************
 *            NETCFG             *
 *********************************
 */
/* Triggered by IPCFG
 */
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_RifCreatedCallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When RIF is created, IPCFG will notify this event.
 *
 * INPUT:
 *      src_csc_id -- The csc_id(SYS_MODULE_STKTPLG defined in sys_module.h) who
 *                    triggers this event
 *      ifindex
 *      addr_p      -- the pointer to the ip and mask of the rif in the L_INET_AddrIp_T type.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_RifCreatedCallBack(UI32_T src_csc_id, UI32_T ifindex, L_INET_AddrIp_T *addr_p)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_RifCreated_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                       *syscb_msg_p;
    SYS_CALLBACK_MGR_RifCreated_CBData_T         *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_RifCreated_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_RifCreated_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_CREATED;
    cbdata_msg_p->ifindex = ifindex;
    memcpy(&(cbdata_msg_p->addr), addr_p, sizeof(cbdata_msg_p->addr));
    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_RifActiveCallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When RIF is activated, IPCFG will notify this event.
 *
 * INPUT:
 *      src_csc_id -- The csc_id(SYS_MODULE_STKTPLG defined in sys_module.h) who
 *                    triggers this event
 *      ifindex
 *      addr_p      -- the pointer to the ip and mask of the rif in the L_INET_AddrIp_T type.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_RifActiveCallBack(UI32_T src_csc_id, UI32_T ifindex, L_INET_AddrIp_T *addr_p)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_RifActive_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                       *syscb_msg_p;
    SYS_CALLBACK_MGR_RifActive_CBData_T          *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_RifActive_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_RifActive_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_ACTIVE;
    cbdata_msg_p->ifindex = ifindex;
    memcpy(&(cbdata_msg_p->addr), addr_p, sizeof(cbdata_msg_p->addr));
    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_RifDownCallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When RIF is down, IPCFG will notify this event.
 *
 * INPUT:
 *      src_csc_id -- The csc_id(SYS_MODULE_STKTPLG defined in sys_module.h) who
 *                    triggers this event
 *      ifindex
 *      addr_p      -- the pointer to the ip and mask of the rif in the L_INET_AddrIp_T type.
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_RifDownCallBack(UI32_T src_csc_id, UI32_T ifindex, L_INET_AddrIp_T *addr_p)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_RifDown_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                       *syscb_msg_p;
    SYS_CALLBACK_MGR_RifDown_CBData_T         *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_RifDown_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_RifDown_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_DOWN;
    cbdata_msg_p->ifindex = ifindex;
    memcpy(&(cbdata_msg_p->addr), addr_p, sizeof(cbdata_msg_p->addr));
    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_RifDestroyedCallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When RIF is destroyed, IPCFG will notify this event.
 *
 * INPUT:
 *      src_csc_id -- The csc_id(SYS_MODULE_STKTPLG defined in sys_module.h) who
 *                    triggers this event
 *      addr_p      -- the pointer to the ip and mask of the rif in the L_INET_AddrIp_T type.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_RifDestroyedCallBack(UI32_T src_csc_id, UI32_T ifindex, L_INET_AddrIp_T *addr_p)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_RifDestroyed_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                       *syscb_msg_p;
    SYS_CALLBACK_MGR_RifDestroyed_CBData_T         *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_RifDestroyed_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_RifDestroyed_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_DESTROYED;
    cbdata_msg_p->ifindex = ifindex;
    memcpy(&(cbdata_msg_p->addr), addr_p, sizeof(cbdata_msg_p->addr));
    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*Donny.li modify for VRRP */
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_NETCFG_L3IfDestroy
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Layer 3 interface destroy notificatiion from NETCFG.
 *
 * INPUT:
 *      ifindex     -- index of the L3 interface
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_NETCFG_L3IfDestroy(UI32_T src_csc_id, UI32_T ifindex)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_NETCFG_ifindex_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                       *syscb_msg_p;
    SYS_CALLBACK_MGR_NETCFG_ifindex_CBData_T   *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_NETCFG_ifindex_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_NETCFG_ifindex_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_L3IF_DESTROY;
    cbdata_msg_p->ifindex = ifindex;
    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}
/*Donny.li modify for VRRP end*/


#if (SYS_CPNT_DHCPV6 == TRUE)
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_NETCFG_IPv6AddrAutoConfig
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      IPv6 address autoconfig enable/disable notification from NETCFG.
 *
 * INPUT:
 *      ifindex     -- the L3 interface which IPv6 address is configured manually.
 *      status      -- the status of IPv6AddrAutoconfig
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_NETCFG_IPv6AddrAutoConfig(UI32_T src_csc_id, UI32_T ifindex, BOOL_T status)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_NETCFG_ifindex_bool_v_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                       *syscb_msg_p;
    SYS_CALLBACK_MGR_NETCFG_ifindex_bool_v_CBData_T   *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_NETCFG_ifindex_bool_v_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_NETCFG_ifindex_bool_v_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_IPV6_ADDRAUTOCONFIG;
    cbdata_msg_p->ifindex = ifindex;
    cbdata_msg_p->bool_v = status;
    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_NETCFG_L3IfCreate
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Layer 3 interface creation notification from NETCFG.
 *
 * INPUT:
 *      ifindex     -- index of the L3 interface
 *      status      -- the status of IPv6AddrAutoconfig
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_NETCFG_L3IfCreate(UI32_T src_csc_id, UI32_T ifindex, BOOL_T status)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_NETCFG_ifindex_bool_v_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                       *syscb_msg_p;
    SYS_CALLBACK_MGR_NETCFG_ifindex_bool_v_CBData_T   *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_NETCFG_ifindex_bool_v_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_NETCFG_ifindex_bool_v_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_L3IF_CREATE;
    cbdata_msg_p->ifindex = ifindex;
    cbdata_msg_p->bool_v = status;
    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}



/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_NETCFG_L3IfOperStatusUp
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Layer 3 interface operation status up notification from NETCFG.
 *
 * INPUT:
 *      ifindex     -- index of the L3 interface
 *      status      -- the status of IPv6AddrAutoconfig
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_NETCFG_L3IfOperStatusUp(UI32_T src_csc_id, UI32_T ifindex, BOOL_T status)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_NETCFG_ifindex_bool_v_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                       *syscb_msg_p;
    SYS_CALLBACK_MGR_NETCFG_ifindex_bool_v_CBData_T   *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_NETCFG_ifindex_bool_v_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_NETCFG_ifindex_bool_v_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_L3IF_OPER_STATUS_UP;
    cbdata_msg_p->ifindex = ifindex;
    cbdata_msg_p->bool_v = status;
    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_NETCFG_L3IfOperStatusDown
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Layer 3 interface operation status down notificatiion from NETCFG.
 *
 * INPUT:
 *      ifindex     -- index of the L3 interface
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_NETCFG_L3IfOperStatusDown(UI32_T src_csc_id, UI32_T ifindex)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_NETCFG_ifindex_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                       *syscb_msg_p;
    SYS_CALLBACK_MGR_NETCFG_ifindex_CBData_T   *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_NETCFG_ifindex_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_NETCFG_ifindex_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_L3IF_OPER_STATUS_DOWN;
    cbdata_msg_p->ifindex = ifindex;
    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}
#endif

/*********************************
 *            CLI                *
 *********************************
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_MGR_ProvisionCompleteCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When provision complete , CLI will notify other CSC groups by this
 *      function.
 *
 * INPUT:
 *      src_csc_id -- The csc_id(SYS_MODULE_CLI defined in sys_module.h) who
 *                    triggers this event
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ProvisionCompleteCallback(UI32_T src_csc_id)
{
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(0))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(0);
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_PROVISION_COMPLETE;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_MGR_ModuleProvisionCompleteCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When module provision complete , CLI will notify other CSC groups by this
 *      function.
 *
 * INPUT:
 *      src_csc_id -- The csc_id(SYS_MODULE_CLI defined in sys_module.h) who
 *                    triggers this event
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ModuleProvisionCompleteCallback(UI32_T src_csc_id,UI32_T unit)
{
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_ModuleStateChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_ModuleStateChanged_CBData_T         *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_ModuleStateChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_MODULE_PROVISION_COMPLETE;
    cbdata_msg_p= (SYS_CALLBACK_MGR_ModuleStateChanged_CBData_T*)syscb_msg_p->callback_data;
    cbdata_msg_p->unit_id= unit;
    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_MGR_EnterTransitionModeCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When enter transition mode , CLI will notify other CSC groups by this
 *      function.
 *
 * INPUT:
 *      src_csc_id -- The csc_id(SYS_MODULE_CLI defined in sys_module.h) who
 *                    triggers this event
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_EnterTransitionModeCallback(UI32_T src_csc_id)
{
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(0))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(0);
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_ENTER_TRANSITION_MODE;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*********************************
 *            LLDP               *
 *********************************
 */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_TelephoneDetectCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Notify other CSC when detecting a new neighbor.
 *
 * INPUT   : src_csc_id           -- The csc_id who triggers this event (SYS_MODULE_LLDP)
 *           lport                --
 *           mac_addr             --
 *           network_addr_subtype --
 *           network_addr         --
 *           network_addr_len     --
 *           network_addr_ifindex --
 *           tel_exist            --
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_TelephoneDetectCallback(UI32_T src_csc_id,
                                                UI32_T lport,
                                                UI8_T *mac_addr,
                                                UI8_T network_addr_subtype,
                                                UI8_T *network_addr,
                                                UI8_T network_addr_len,
                                                UI32_T network_addr_ifindex,
                                                BOOL_T tel_exist)
{
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_TelephoneDetect_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_TelephoneDetect_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_TelephoneDetect_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_TelephoneDetect_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_TELEPHONE_DETECT;
    cbdata_msg_p->lport = lport;
    memcpy(cbdata_msg_p->mac_addr, mac_addr, sizeof(cbdata_msg_p->mac_addr));
    cbdata_msg_p->network_addr_subtype = network_addr_subtype;
    memcpy(cbdata_msg_p->network_addr, network_addr, sizeof(cbdata_msg_p->network_addr));
    cbdata_msg_p->network_addr_len = network_addr_len;
    cbdata_msg_p->network_addr_ifindex = network_addr_ifindex;
    cbdata_msg_p->tel_exist = tel_exist;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

#if (SYS_CPNT_POE == TRUE)
#ifdef SYS_CPNT_POE_PSE_DOT3AT
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_ReceiveDot3atInfoCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Notify other CSC when LLDP receives PoE Dot3at related TLVs.
 *
 * INPUT   : src_csc_id               -- The csc_id who triggers this event (SYS_MODULE_LLDP)
 *           unit                     --
 *           port                     --
 *           power_type               --
 *           power_source             --
 *           power_priority           --
 *           power_value              --
 *           requested_power_type     --
 *           requested_power_source   --
 *           requested_power_priority --
 *           requested_power_value    --
 *           acknowledge              --
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveDot3atInfoCallback(UI32_T  src_csc_id,
                                                  UI32_T  unit,
                                                  UI32_T  port,
                                                  UI8_T   power_type,
                                                  UI8_T   power_source,
                                                  UI8_T   power_priority,
                                                  UI16_T  power_value,
                                                  UI8_T   requested_power_type,
                                                  UI8_T   requested_power_source,
                                                  UI8_T   requested_power_priority,
                                                  UI16_T  requested_power_value,
                                                  UI8_T   acknowledge)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T *syscb_msg_p;
    SYS_CALLBACK_MGR_Dot3atInfoReceived_CBData_T *cbdata_msg_p;
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_Dot3atInfoReceived_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_Dot3atInfoReceived_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p = (SYS_CALLBACK_MGR_Dot3atInfoReceived_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_DOT3AT_INFO_RECEIVED;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;
    cbdata_msg_p->power_type = power_type;
    cbdata_msg_p->power_source = power_source;
    cbdata_msg_p->power_priority = power_priority;
    cbdata_msg_p->power_value = power_value;
    cbdata_msg_p->requested_power_type = requested_power_type;
    cbdata_msg_p->requested_power_source = requested_power_source;
    cbdata_msg_p->requested_power_priority = requested_power_priority;
    cbdata_msg_p->requested_power_value = requested_power_value;
    cbdata_msg_p->acknowledge = acknowledge;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_Dot3atInfoReceivedCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Notify other CSC when LLDP receives DTE Power via MDI TLV defined
 *           in 802.3at.
 *
 * INPUT   : src_csc_id          -- The csc_id who triggers this event (SYS_MODULE_LLDP)
 *           unit                --
 *           port                --
 *           power_type          --
 *           power_source        --
 *           power_priority      --
 *           pd_requested_power  --
 *           pse_allocated_power --
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_Dot3atInfoReceivedCallback(UI32_T   src_csc_id,
                                                  UI32_T   unit,
                                                  UI32_T   port,
                                                  UI8_T    power_type,
                                                  UI8_T    power_source,
                                                  UI8_T    power_priority,
                                                  UI16_T   pd_requested_power,
                                                  UI16_T   pse_allocated_power)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T *syscb_msg_p;
    SYS_CALLBACK_MGR_Dot3atInfoReceived_CBData_T *cbdata_msg_p;
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_Dot3atInfoReceived_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_Dot3atInfoReceived_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p = (SYS_CALLBACK_MGR_Dot3atInfoReceived_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_DOT3AT_INFO_RECEIVED;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;
    cbdata_msg_p->power_type = power_type;
    cbdata_msg_p->power_source = power_source;
    cbdata_msg_p->power_priority = power_priority;
    cbdata_msg_p->pd_requested_power = pd_requested_power;
    cbdata_msg_p->pse_allocated_power = pse_allocated_power;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}
#endif
#endif
#endif

#if (SYS_CPNT_CN == TRUE)
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_CnRemoteChangeCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Notify CN of remote CN data change
 *
 * INPUT:
 *      src_csc_id       -- the csc_id(SYS_MODULE_LLDP) who triggers this event
 *      lport            -- the logical port which receives the CN TLV
 *      neighbor_num     -- the number of neighbors
 *      cnpv_indicators  -- the CNPV indicators in the received CN TLV
 *      ready_indicators -- the Ready indicators in the received CN TLV
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_CnRemoteChangeCallback(UI32_T src_csc_id, UI32_T lport,
        UI32_T neighbor_num, UI8_T cnpv_indicators, UI8_T ready_indicators)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T *syscb_msg_p;
    SYS_CALLBACK_MGR_CnRemoteChange_CBData_T *cbdata_msg_p;
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_CnRemoteChange_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_CnRemoteChange_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p = (SYS_CALLBACK_MGR_CnRemoteChange_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_CN_REMOTE_CHANGE;
    cbdata_msg_p->lport = lport;
    cbdata_msg_p->neighbor_num = neighbor_num;
    cbdata_msg_p->cnpv_indicators = cnpv_indicators;
    cbdata_msg_p->ready_indicators = ready_indicators;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE);
}
#endif /* #if (SYS_CPNT_CN == TRUE) */

/*********************************
 *            PRIMGMT            *
 *********************************
 */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_TelephoneDetectCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when cos mapping changed.
 *
 * INPUT   : src_csc_id    -- The csc_id who triggers this event (SYS_MODULE_PRIMGMT)
 *           lport_ifindex -- specify which port the event occured
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_CosChangedCallback(UI32_T src_csc_id, UI32_T lport_ifindex)
{
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_CosChanged_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_CosChanged_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_CosChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_CosChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_COS_CHANGED;
    cbdata_msg_p->lport_ifindex = lport_ifindex;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*********************************
 *            RADIUS             *
 *********************************
 */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_AnnounceRADIUSPacket
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when radius packet is received.
 * INPUT   :
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_AnnounceRADIUSPacket(
    UI32_T  src_csc_id,     UI32_T  dest_msgq_key,  UI32_T  result,
    UI8_T   *data_buf,      UI32_T  data_len,       UI32_T  src_port,
    UI8_T   *src_mac,       UI32_T  src_vid,
    char    *authorized_vlan_list,  char   *authorized_qos_list,
    UI32_T  session_timeout,        UI32_T  server_ip)
{
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_AnnounceRADIUSPacket_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AnnounceRADIUSPacket_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AnnounceRADIUSPacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_AnnounceRADIUSPacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RADIUS_PACKET;
    cbdata_msg_p->result = result;
    cbdata_msg_p->data_len = data_len;
    cbdata_msg_p->src_port = src_port;
    cbdata_msg_p->src_vid           = src_vid;
    cbdata_msg_p->session_timeout = session_timeout;
    cbdata_msg_p->server_ip         = server_ip;

    memcpy(cbdata_msg_p->src_mac,  src_mac,  sizeof(cbdata_msg_p->src_mac));
    if (NULL != data_buf)
    {
        memcpy(cbdata_msg_p->data_buf, data_buf, data_len);
    }
    memcpy(cbdata_msg_p->authorized_vlan_list, authorized_vlan_list, sizeof(cbdata_msg_p->authorized_vlan_list));
    memcpy(cbdata_msg_p->authorized_qos_list, authorized_qos_list, sizeof(cbdata_msg_p->authorized_qos_list));

    return SYS_CALLBACK_MGR_SendMsgWithMsgQKey(src_csc_id, dest_msgq_key, sysfun_msg_p, FALSE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_AnnounceRadiusAuthorizedResult
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when radius authoirized result is received.
 * INPUT   :
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_AnnounceRadiusAuthorizedResult(
    UI32_T  src_csc_id,             UI32_T  dest_msgq_key,
    UI32_T  lport,                  UI8_T   *mac,
    int     identifier,             BOOL_T  authorized_result,
    UI8_T   *authorized_vlan_list,  UI8_T   *authorized_qos_list,
    UI32_T  session_time,           UI32_T  server_ip)
{
    SYSFUN_Msg_T                                    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                          *syscb_msg_p;
    SYS_CALLBACK_MGR_AnnounceRadaAuthRes_CBData_T   *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AnnounceRadaAuthRes_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AnnounceRadaAuthRes_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_AnnounceRadaAuthRes_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id  = SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RADA_AUTH_RESULT;
    cbdata_msg_p->lport             = lport;
    cbdata_msg_p->identifier        = identifier;
    cbdata_msg_p->authorized_result = authorized_result;
    cbdata_msg_p->session_time      = session_time;
    cbdata_msg_p->server_ip         = server_ip;
    memcpy(cbdata_msg_p->authorized_mac, mac, sizeof(cbdata_msg_p->authorized_mac));
    memcpy(cbdata_msg_p->authorized_vlan_list, authorized_vlan_list, sizeof(cbdata_msg_p->authorized_vlan_list));
    memcpy(cbdata_msg_p->authorized_qos_list,  authorized_qos_list, sizeof(cbdata_msg_p->authorized_qos_list));

    return SYS_CALLBACK_MGR_SendMsgWithMsgQKey(src_csc_id, dest_msgq_key, sysfun_msg_p, FALSE);
}

#if (SYS_CPNT_IGMPAUTH == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_AnnounceRadiusIGMPAuthResult
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when radius IGMP authentication result is
 *           received.
 * INPUT   : src_csc_id - SYS_MODULE_RADIUS
 *           result     - IGMP authentication result (RADIUS_RESULT_FAIL ||
 *                                                    RADIUS_RESULT_SUCCESS ||
 *                                                    RADIUS_RESULT_TIMEOUT)
 *           auth_port  - user port
 *           ip_address - multicast group ip
 *           auth_mac   - user MAC address
 *           vlan_id    - vlan id
 *           src_ip     - user ip
 *           msg_type   - igmp version
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T
SYS_CALLBACK_MGR_AnnounceRadiusIGMPAuthResult(
    UI32_T   src_csc_id,
    UI32_T   result,
    UI32_T   auth_port,
    UI32_T   ip_address,
    UI8_T    *auth_mac,
    UI32_T   vlan_id,
    UI32_T   src_ip,
    UI8_T    msg_type)
{
    SYSFUN_Msg_T            *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_AnnounceRadiusIGMPAuthResult_CBData_T  *cbdata_msg_p;
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
              sizeof(SYS_CALLBACK_MGR_AnnounceRadiusIGMPAuthResult_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AnnounceRadiusIGMPAuthResult_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p = (SYS_CALLBACK_MGR_AnnounceRadiusIGMPAuthResult_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id =
        SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RADIUS_IGMPAUTH_RESULT;
    cbdata_msg_p->result = result;
    cbdata_msg_p->auth_port = auth_port;
    cbdata_msg_p->ip_address = ip_address;
    cbdata_msg_p->vlan_id = vlan_id;
    cbdata_msg_p->src_ip = src_ip;
    cbdata_msg_p->msg_type = msg_type;
    memcpy(cbdata_msg_p->auth_mac,  auth_mac,  sizeof(cbdata_msg_p->auth_mac));

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE);
}
#endif /* #if (SYS_CPNT_IGMPAUTH == TRUE) */

/*maggie liu for RADIUS authentication ansync*/
BOOL_T SYS_CALLBACK_MGR_AnnounceRADIUSAuthenResult(UI32_T  src_csc_id,
    UI32_T  cookie, I32_T  result, I32_T privilege)
{
    SYSFUN_Msg_T                                    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                          *syscb_msg_p;
    SYS_CALLBACK_MGR_AnnounceRADIUSAuthenRadius_CBData_T   *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AnnounceRADIUSAuthenRadius_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AnnounceRADIUSAuthenRadius_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_AnnounceRADIUSAuthenRadius_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id  = SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RADIUS_AUTH_RESULT;
    cbdata_msg_p->result = result;
    cbdata_msg_p->privilege = privilege;
    cbdata_msg_p->cookie = cookie;

     return  SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

BOOL_T SYS_CALLBACK_MGR_AnnounceRemServerAuthResult(
    UI32_T  src_csc_id,
    I32_T result,
    UI32_T privilege,
    void *cookie,
    UI32_T cookie_size)
{
    SYSFUN_Msg_T                                            *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                                  *syscb_msg_p;
    SYS_CALLBACK_MGR_AnnounceRADIUSAuthenRadius_2_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AnnounceRADIUSAuthenRadius_2_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AnnounceRADIUSAuthenRadius_2_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_AnnounceRADIUSAuthenRadius_2_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id  = SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RADIUS_AUTH_RESULT_2;

    if (sizeof(cbdata_msg_p->cookie) < cookie_size)
    {
        printf("%s (%d): Size of cookie is too small!!\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    cbdata_msg_p->result = result;
    cbdata_msg_p->privilege = privilege;

    memcpy(cbdata_msg_p->cookie, cookie, cookie_size);
    cbdata_msg_p->cookie_size = cookie_size;

    return  SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*********************************
 *            DOT1X              *
 *********************************
 */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_AnnounceDot1xAuthorizedResult
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when dot1x authorized result is received.
 * INPUT   :
 * OUTPUT  : None.
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_AnnounceDot1xAuthorizedResult(
    UI32_T  src_csc_id,     UI32_T  dest_msgq_key,  UI32_T  lport,
    UI8_T   *port_mac,      UI32_T  eap_identifier, UI32_T  auth_result,
    char    *authorized_vlan_list,  char    *authorized_qos_list,
    UI32_T  session_time,   UI32_T  server_ip)
{
    SYSFUN_Msg_T                                    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                          *syscb_msg_p;
    SYS_CALLBACK_MGR_AnnounceDot1xAuthRes_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AnnounceDot1xAuthRes_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AnnounceDot1xAuthRes_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_AnnounceDot1xAuthRes_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id  = SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_DOT1X_AUTH_RESULT;
    cbdata_msg_p->lport             = lport;
    cbdata_msg_p->authorized_result = auth_result;
    cbdata_msg_p->eap_identifier    = eap_identifier;
    cbdata_msg_p->session_time      = session_time;
    cbdata_msg_p->server_ip         = server_ip;

    memcpy(cbdata_msg_p->authorized_mac,  port_mac,  sizeof(cbdata_msg_p->authorized_mac));

    if (NULL != authorized_vlan_list)
    {
        memcpy(cbdata_msg_p->authorized_vlan_list, authorized_vlan_list, sizeof(cbdata_msg_p->authorized_vlan_list));
    }
    else
    {
        memset(cbdata_msg_p->authorized_vlan_list,  0,  sizeof(cbdata_msg_p->authorized_vlan_list));
    }

    if (NULL != authorized_qos_list)
    {
        memcpy(cbdata_msg_p->authorized_qos_list,  authorized_qos_list,  sizeof(cbdata_msg_p->authorized_qos_list));
    }
    else
    {
        memset(cbdata_msg_p->authorized_qos_list,  0,  sizeof(cbdata_msg_p->authorized_qos_list));
    }

    return SYS_CALLBACK_MGR_SendMsgWithMsgQKey(src_csc_id, dest_msgq_key, sysfun_msg_p, FALSE);
}

/*********************************
 *            XFER               *
 *********************************
 */
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_AnnounceXferResultCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When XFER function is done , XFER will notify this event.
 *
 * INPUT:
 *      src_csc_id -- SYS_MODULE_XFER
 *      dest_msgq  -- the destination message queue.
 *      fun        -- the callback function pointer.
 *      arg_cookie -- the callback function argument.
 *      arg_status -- the callback function argument.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */

BOOL_T SYS_CALLBACK_MGR_AnnounceXferResultCallback(UI32_T src_csc_id, UI32_T dest_msgq, void *fun, void *arg_cookie, UI32_T arg_status)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_AnnounceXferResult_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_AnnounceXferResult_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AnnounceXferResult_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_AnnounceXferResult_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_XFER_RESULT;
    //syscb_msg_p->cookie = fun;
    syscb_msg_p->cookie = 0;
    cbdata_msg_p->cookie = arg_cookie;
    cbdata_msg_p->status = arg_status;

    return SYS_CALLBACK_MGR_SendMsgWithMsgQKey(src_csc_id, dest_msgq, sysfun_msg_p, TRUE);
}

/*********************************
 *         HTTP SSHD             *
 *********************************
 */
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_AnnounceCliXferResultCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When XFER function is done , XFER will notify this event.
 *
 * INPUT:
 *      src_csc_id -- SYS_MODULE_HTTP/SYS_MODULE_SSH
 *      dest_msgq  -- the destination message queue.
 *      fun        -- the callback function pointer.
 *      arg_cookie -- the callback function argument.
 *      arg_status -- the callback function argument.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */

BOOL_T SYS_CALLBACK_MGR_AnnounceCliXferResultCallback(UI32_T src_csc_id, UI32_T dest_msgq, void *fun, void *arg_cookie, UI32_T arg_status)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_AnnounceCliXferResult_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_AnnounceCliXferResult_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AnnounceCliXferResult_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_AnnounceCliXferResult_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_CLI_XFER_RESULT;
    //syscb_msg_p->cookie = fun;
    syscb_msg_p->cookie = 0;
    cbdata_msg_p->cookie = arg_cookie;
    cbdata_msg_p->status = arg_status;

    return SYS_CALLBACK_MGR_SendMsgWithMsgQKey(src_csc_id, dest_msgq, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_AnnounceAclDeleted
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when one ACL be deleted.
 * INPUT   :
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_AnnounceAclDeleted(
        UI32_T      src_csc_id,
        const char  *acl_name,
        UI32_T      acl_type,
        UI8_T       dynamic_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST])
{
    enum {SIZE_OF_PARAM = sizeof(SYS_CALLBACK_MGR_AnnounceAclDeleted_CBData_T)};

    SYSFUN_Msg_T                                    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                          *syscb_msg_p;
    SYS_CALLBACK_MGR_AnnounceAclDeleted_CBData_T     *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(SIZE_OF_PARAM))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(SIZE_OF_PARAM);
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_AnnounceAclDeleted_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_ACL_DELETED;
    cbdata_msg_p->acl_type = acl_type;
    memcpy(cbdata_msg_p->acl_name, acl_name, sizeof(cbdata_msg_p->acl_name));
    memcpy(cbdata_msg_p->dynamic_port_list, dynamic_port_list, sizeof(cbdata_msg_p->dynamic_port_list));

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_AnnouncePolicyMapDeleted
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when one ACL be deleted.
 * INPUT   :
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_AnnouncePolicyMapDeleted(
        UI32_T      src_csc_id,
        const char  *policy_map_name,
        UI8_T       dynamic_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST])
{
    enum {SIZE_OF_PARAM = sizeof(SYS_CALLBACK_MGR_AnnouncePolicyMapDeleted_CBData_T)};

    SYSFUN_Msg_T                                    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                          *syscb_msg_p;
    SYS_CALLBACK_MGR_AnnouncePolicyMapDeleted_CBData_T     *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(SIZE_OF_PARAM))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(SIZE_OF_PARAM);
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_AnnouncePolicyMapDeleted_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_POLICY_MAP_DELETED;
    memcpy(cbdata_msg_p->policy_map_name, policy_map_name, sizeof(cbdata_msg_p->policy_map_name));
    memcpy(cbdata_msg_p->dynamic_port_list, dynamic_port_list, sizeof(cbdata_msg_p->dynamic_port_list));

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*********************************
 *         CoS                   *
 *********************************
 */
#if (SYS_CPNT_COS == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_AnnounceCosPortConfigChanged
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when the COS config of a port changed
 *
 * INPUT   : src_csc_id         -- The csc_id who triggers this event
 *           l_port             -- Logic port
 *           priority_of_config -- the config is changed by which priority(user,..)
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_AnnounceCosPortConfigChanged(
        UI32_T src_csc_id,
        UI32_T l_port,
        UI32_T priority_of_config)
{
    enum {SIZE_OF_PARAM = sizeof(SYS_CALLBACK_MGR_AnnounceCosPortConfigChanged_CBData_T)};

    SYSFUN_Msg_T                                            *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                                  *syscb_msg_p;
    SYS_CALLBACK_MGR_AnnounceCosPortConfigChanged_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(SIZE_OF_PARAM))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(SIZE_OF_PARAM);
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_AnnounceCosPortConfigChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_COS_PORT_CONFIG_CHANGED;
    cbdata_msg_p->l_port = l_port;
    cbdata_msg_p->priority_of_config = priority_of_config;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

#endif /* #if (SYS_CPNT_COS == TRUE) */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_NextHopStatusChangeCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : The registered callback is invoked when a host route (as a nexthop of a route)
 *           status change.
 *
 * INPUT   : src_csc_id -- The csc_id who triggers this event (SYS_MODULE_AMTRL3)
 *           action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                             AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id          -- FIB id
 *           status          -- status of this host route (unresolved, ready...)
 *           ip_addr         -- dst ip of this host route
 *           lport_ifindex   -- lport
 *           vid_ifindex     -- vid if index
 *           dst_mac         -- dst mac of this host route
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
#if (SYS_CPNT_AMTRL3 == TRUE)
#if 0
BOOL_T SYS_CALLBACK_MGR_NextHopStatusChangeCallback(UI32_T src_csc_id, UI32_T action_flags, UI32_T fib_id, UI32_T status, IpAddr_T ip_addr, UI32_T lport_ifindex, UI32_T vid_ifindex, UI8_T *dst_mac)
{
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_Nexthop_Status_Change_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_Nexthop_Status_Change_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_Nexthop_Status_Change_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_Nexthop_Status_Change_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_NEXTHOP_STATUS_CHANGE;
    cbdata_msg_p->action_flags = action_flags;
    cbdata_msg_p->fib_id= fib_id;
    cbdata_msg_p->status= status;
    cbdata_msg_p->ip_addr= ip_addr;
    cbdata_msg_p->lport_ifindex= lport_ifindex;
    cbdata_msg_p->vid_ifindex= vid_ifindex;
    memcpy(cbdata_msg_p->dst_mac, dst_mac, sizeof(UI8_T) * SYS_ADPT_MAC_ADDR_LEN);

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}
#endif
#endif


#if (SYS_CPNT_PPPOE_IA == TRUE)
/*********************************
 *          PPPOE IA             *
 *********************************
 */
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceivePppoedPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When LAN received packet for pppoe ia, it will notify pppoe ia with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      tag_info       --- packet tag info
 *      type           --- packet type
 *      pkt_length     --- packet length
 *      lport          --- source lport id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      1. for PPPOE IA
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceivePppoedPacketCallback(
    UI32_T              src_csc_id,
    L_MM_Mref_Handle_T  *mref_handle_p,
    UI8_T               dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T               src_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI16_T              tag_info,
    UI16_T              type,
    UI32_T              pkt_length,
    UI32_T              lport)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_PPPOED_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->tag_info           = tag_info;
    cbdata_msg_p->type               = type;
    cbdata_msg_p->pkt_length         = pkt_length;
    cbdata_msg_p->lport              = lport;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE))
    {
        L_MM_Mref_Release(&mref_handle_p);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
#endif /* #if (SYS_CPNT_PPPOE_IA == TRUE) */

/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_MGR_HandleIPCMsgAndCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Handle the sys_callback_mgr ipc message and do callback.
 *
 * INPUT:
 *      ipcmsg_p        -- sys_callback_mgr ipc message to be handled
 *      callback_fun    -- callback function to be executed
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  The callback message has been handled
 *      FALSE --  Cannot recognize the callback message.
 *
 * NOTES:
 *      1. Each CSC group shall have a callback handler to deal with all callbacks
 *         within the CSC group.
 *      2. The callback handler of the CSC group shall identify the callback function
 *         to be called through callback_event_id in SYS_CALLBACK_MGR_Msg_T.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(SYSFUN_Msg_T* ipcmsg_p, void* callback_fun)
{
    SYS_CALLBACK_MGR_Msg_T *syscallback_msg_p;
    BOOL_T ret;

    if(ipcmsg_p==NULL)
        return FALSE;

    syscallback_msg_p = (SYS_CALLBACK_MGR_Msg_T*)ipcmsg_p->msg_buf;

    switch(syscallback_msg_p->callback_event_id)
    {
        /* AMTR */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_INTRUSION_MAC:
        {
            SYS_CALLBACK_MGR_IntrusionMac_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_IntrusionMac_CBData_T *cb_data_p;

            cb_data_p = (SYS_CALLBACK_MGR_IntrusionMac_CBData_T*)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_IntrusionMac_CBFun_T)callback_fun;

            (*cb_fun)(cb_data_p->src_lport, cb_data_p->vid, cb_data_p->src_mac, cb_data_p->dst_mac, cb_data_p->ether_type);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_MOVE:
        {
            SYS_CALLBACK_MGR_PortMove_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_PortMove_CBData_T *cb_data_p;

            cb_data_p = (SYS_CALLBACK_MGR_PortMove_CBData_T*)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_PortMove_CBFun_T)callback_fun;

            (*cb_fun)(cb_data_p->number_of_entry, cb_data_p->buf);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SECURITY_PORT_MOVE:
        {
            SYS_CALLBACK_MGR_SecurityPortMove_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_SecurityPortMove_CBData_T *cb_data_p;

            cb_data_p = (SYS_CALLBACK_MGR_SecurityPortMove_CBData_T*)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_SecurityPortMove_CBFun_T)callback_fun;

            (*cb_fun)(cb_data_p->ifindex, cb_data_p->vid, cb_data_p->mac, cb_data_p->original_ifindex);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_OVS_MAC_UPDATE:
        {
            SYS_CALLBACK_MGR_MacNotify_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_MacNotify_CBData_T *cb_data_p;

            cb_data_p = (SYS_CALLBACK_MGR_MacNotify_CBData_T*)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_MacNotify_CBFun_T)callback_fun;

            (*cb_fun)(cb_data_p->ifindex, cb_data_p->vid, cb_data_p->mac, cb_data_p->is_add);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MLAG_MAC_UPDATE:
        {
            SYS_CALLBACK_MGR_MacNotify_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_MacNotify_CBData_T *cb_data_p;

            cb_data_p = (SYS_CALLBACK_MGR_MacNotify_CBData_T*)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_MacNotify_CBFun_T)callback_fun;

            (*cb_fun)(cb_data_p->ifindex, cb_data_p->vid, cb_data_p->mac, cb_data_p->is_add);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_ADDR_UPDATE:
        {
            SYS_CALLBACK_MGR_MacNotify_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_MacNotify_CBData_T *cb_data_p;

            cb_data_p = (SYS_CALLBACK_MGR_MacNotify_CBData_T*)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_MacNotify_CBFun_T)callback_fun;

            (*cb_fun)(cb_data_p->ifindex, cb_data_p->vid, cb_data_p->mac, cb_data_p->is_add);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_AUTO_LEARN:
        {
            SYS_CALLBACK_MGR_AutoLearn_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_AutoLearn_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_AutoLearn_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_AutoLearn_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->ifindex, cb_data_p->portsec_status);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_TABLE_DELETE_BY_PORT:
        {
            SYS_CALLBACK_MGR_MACTableDeleteByPort_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_MACTableDeleteByPort_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_MACTableDeleteByPort_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_MACTableDeleteByPort_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->ifindex, cb_data_p->reason);
            ret = TRUE;

        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_TABLE_DELETE_BY_VID:
        {
            SYS_CALLBACK_MGR_MACTableDeleteByVid_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_MACTableDeleteByVid_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_MACTableDeleteByVid_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_MACTableDeleteByVid_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->vid);
            ret = TRUE;

        }
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_TABLE_DELETE_BY_VID_AND_PORT:
        {
            SYS_CALLBACK_MGR_MACTableDeleteByVIDnPort_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_MACTableDeleteByVIDnPort_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_MACTableDeleteByVIDnPort_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_MACTableDeleteByVIDnPort_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->vid, cb_data_p->ifindex);
            ret = TRUE;
        }
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_TABLE_DELETE_BY_LIFE_TIME:
        {
            SYS_CALLBACK_MGR_MacTableDeleteByLifeTime_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_MacTableDeleteByLifeTime_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_MacTableDeleteByLifeTime_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_MacTableDeleteByLifeTime_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->life_time);
            ret = TRUE;

        }
            break;


        /* VLAN */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_CREATE:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_CREATE_FOR_GVRP:
        {
            SYS_CALLBACK_MGR_VlanCreate_CBFun_T     cb_fun;
            SYS_CALLBACK_MGR_VlanCreate_CBData_T    *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_VlanCreate_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_VlanCreate_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->vid_ifindex, cb_data_p->vlan_status);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_DESTROY:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_DESTROY_FOR_GVRP:
        {
            SYS_CALLBACK_MGR_VlanDestroy_CBFun_T    cb_fun;
            SYS_CALLBACK_MGR_VlanDestroy_CBData_T   *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_VlanDestroy_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_VlanDestroy_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->vid_ifindex, cb_data_p->vlan_status);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_L3VLAN_DESTROY:
        {
            SYS_CALLBACK_MGR_VlanDestroy_CBFun_T    cb_fun;
            SYS_CALLBACK_MGR_VlanDestroy_CBData_T   *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_VlanDestroy_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_VlanDestroy_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->vid_ifindex, cb_data_p->vlan_status);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_ADD:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_ADD_FOR_GVRP:
        {
            SYS_CALLBACK_MGR_VlanMemberAdd_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_VlanMemberAdd_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_VlanMemberAdd_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_VlanMemberAdd_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->vid_ifindex, cb_data_p->lport_ifindex, cb_data_p->vlan_status);
            ret = TRUE;
        }
            break;
#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_LIST:
        {
            SYS_CALLBACK_MGR_ProcessList_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_REFINEList_CBData_T  *cb_data_p;
            cb_fun = (SYS_CALLBACK_MGR_ProcessList_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_REFINEList_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(syscallback_msg_p->sub_callback_event_id,cb_data_p);
            ret = TRUE;
        }
        break;
#endif
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE_FOR_GVRP:
        {
            SYS_CALLBACK_MGR_VlanMemberDelete_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_VlanMemberDelete_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_VlanMemberDelete_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_VlanMemberDelete_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->vid_ifindex, cb_data_p->lport_ifindex, cb_data_p->vlan_status);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_PORT_MODE:
        {
            SYS_CALLBACK_MGR_VlanPortMode_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_VlanPortMode_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_VlanPortMode_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_VlanPortMode_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->lport_ifindex, cb_data_p->vlan_port_mode);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_ADD_FIRST_TRUNK_MEMBER:
        {
            SYS_CALLBACK_MGR_VlanFinishAddFirstTrunkMember_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_VlanFinishAddFirstTrunkMember_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_VlanFinishAddFirstTrunkMember_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_VlanFinishAddFirstTrunkMember_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->trunk_ifindex, cb_data_p->member_ifindex);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_ADD_TRUNK_MEMBER:
        {
            SYS_CALLBACK_MGR_VlanFinishAddTrunkMember_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_VlanFinishAddTrunkMember_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_VlanFinishAddTrunkMember_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_VlanFinishAddTrunkMember_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->trunk_ifindex, cb_data_p->member_ifindex);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_DELETE_TRUNK_MEMBER:
        {
            SYS_CALLBACK_MGR_VlanFinishDeleteTrunkMember_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_VlanFinishDeleteTrunkMember_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_VlanFinishDeleteTrunkMember_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_VlanFinishDeleteTrunkMember_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->trunk_ifindex, cb_data_p->member_ifindex);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_DELETE_LAST_TRUNK_MEMBER:
        {
            SYS_CALLBACK_MGR_VlanFinishDeleteLastTrunkMember_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_VlanFinishDeleteLastTrunkMember_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_VlanFinishDeleteLastTrunkMember_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_VlanFinishDeleteLastTrunkMember_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->trunk_ifindex, cb_data_p->member_ifindex);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE_BY_TRUNK:
        {
            SYS_CALLBACK_MGR_VlanMemberDeleteByTrunk_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_VlanMemberDeleteByTrunk_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_VlanMemberDeleteByTrunk_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_VlanMemberDeleteByTrunk_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->vid_ifindex, cb_data_p->lport_ifindex, cb_data_p->vlan_status);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PVID_CHANGE:
        {
            SYS_CALLBACK_MGR_PvidChange_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_PvidChange_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_PvidChange_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_PvidChange_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->lport_ifindex, cb_data_p->old_pvid, cb_data_p->new_pvid);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_L3IF_OPER_STATUS_CHANGED:
        {
            SYS_CALLBACK_MGR_IfOperStatusChanged_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_IfOperStatusChanged_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_IfOperStatusChanged_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_IfOperStatusChanged_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->vid_ifindex, cb_data_p->oper_status);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_ADD_FIRST_TRUNK_MEMBER:
        {
            SYS_CALLBACK_MGR_VlanAddFirstTrunkMember_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_VlanAddFirstTrunkMember_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_VlanAddFirstTrunkMember_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_VlanAddFirstTrunkMember_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->dot1q_vlan_index, cb_data_p->trunk_ifindex, cb_data_p->member_ifindex);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_ADD_TRUNK_MEMBER:
        {
            SYS_CALLBACK_MGR_VlanAddTrunkMember_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_VlanAddTrunkMember_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_VlanAddTrunkMember_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_VlanAddTrunkMember_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->dot1q_vlan_index, cb_data_p->trunk_ifindex, cb_data_p->member_ifindex);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_DELETE_TRUNK_MEMBER:
        {
            SYS_CALLBACK_MGR_VlanDeleteTrunkMember_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_VlanDeleteTrunkMember_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_VlanDeleteTrunkMember_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_VlanDeleteTrunkMember_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->dot1q_vlan_index, cb_data_p->trunk_ifindex, cb_data_p->member_ifindex);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_DELETE_LAST_TRUNK_MEMBER:
        {
            SYS_CALLBACK_MGR_VlanDeleteLastTrunkMember_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_VlanDeleteLastTrunkMember_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_VlanDeleteLastTrunkMember_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_VlanDeleteLastTrunkMember_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->dot1q_vlan_index, cb_data_p->trunk_ifindex, cb_data_p->member_ifindex);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_NAME_CHANGED:
        {
            SYS_CALLBACK_MGR_VlanNameChanged_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_VlanNameChanged_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_VlanNameChanged_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_VlanNameChanged_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->vid);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PROTOVLAN_GID_BINDING_CHANGED:
        {
            SYS_CALLBACK_MGR_ProtovlanGidBindingChanged_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_ProtovlanGidBindingChanged_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_ProtovlanGidBindingChanged_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_ProtovlanGidBindingChanged_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->lport);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_TAG_CHANGED:
        {
            SYS_CALLBACK_MGR_VlanMemberTagChanged_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_VlanMemberTagChanged_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_VlanMemberTagChanged_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_VlanMemberTagChanged_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->vid_ifindex, cb_data_p->lport_ifindex);
            ret = TRUE;
        }
            break;

        /* XSTP */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_LPORT_ENTER_FORWARDING:
        {
            SYS_CALLBACK_MGR_LportEnterForwarding_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_LportEnterForwarding_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_LportEnterForwarding_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_LportEnterForwarding_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->xstid, cb_data_p->lport);
            ret = TRUE;
            break;
        }

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_LPORT_LEAVE_FORWARDING:
        {
            SYS_CALLBACK_MGR_LportLeaveForwarding_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_LportLeaveForwarding_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_LportLeaveForwarding_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_LportLeaveForwarding_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->xstid, cb_data_p->lport);
            ret = TRUE;
            break;
        }

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_LPORT_CHANGE_STATE:
        {
            SYS_CALLBACK_MGR_LportChangeState_CBFun_T   cb_fun;

            cb_fun = (SYS_CALLBACK_MGR_LportChangeState_CBFun_T)callback_fun;

            (*cb_fun)();
            ret = TRUE;
            break;
        }
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_XSTP_LPORT_TC_CHANGE:
        {
            SYS_CALLBACK_MGR_LportTcChange_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_LportTcChange_CBData_T  *cb_data_p;
            cb_data_p = (SYS_CALLBACK_MGR_LportTcChange_CBData_T*)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_LportTcChange_CBFun_T)callback_fun;

            /* to reduce the message size, the vlan bitmap will be retrieved from the
             *  the xstid inside the api but not passed by parameter.
             *
             * Ryan agrees to take the risk that the vlan bitmap retrieved may be not
             *  the same as the one when syscallback is called.
             */
            (*cb_fun)(cb_data_p->is_mstp_mode, cb_data_p->xstid,cb_data_p->lport,cb_data_p->is_root,cb_data_p->tc_timer/*,cb_data_p->instance_vlans_mapped*/);
            ret = TRUE;
            break;
        }
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_STP_CHANGE_VERSION:
        {
            SYS_CALLBACK_MGR_StpChangeVersion_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_StpChangeVersion_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_StpChangeVersion_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_StpChangeVersion_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->mode, cb_data_p->status);
            ret = TRUE;
            break;
        }

        /* AMTRDRV */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_NEW_MAC_ADDRESS:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_AGING_OUT:
        {
            SYS_CALLBACK_MGR_NewMacAddress_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_NewMacAddress_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_NewMacAddress_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_NewMacAddress_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->number_of_entry, cb_data_p->addr_buf);
            ret = TRUE;
            break;
        }

        /* NMTRDRV */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPDATE_LOCAL_NMTRDRV_STATS:
        {
            SYS_CALLBACK_MGR_UpdateLocalNmtrdrvStats_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_UpdateLocalNmtrdrvStats_CBData_T* cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_UpdateLocalNmtrdrvStats_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_UpdateLocalNmtrdrvStats_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->update_type, cb_data_p->unit, cb_data_p->start_port, cb_data_p->port_amount);
            ret = TRUE;
            break;
        }

        /* SWCTRL */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_LPORT_TYPE_CHANGED:
        {
            SYS_CALLBACK_MGR_LPortType_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_LPortType_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_LPortType_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_LPortType_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->ifindex, cb_data_p->port_type);
            ret=TRUE;
        }
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_TYPE_CHANGED:
        {
            SYS_CALLBACK_MGR_UPortType_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_UPortType_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_UPortType_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_UPortType_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->unit, cb_data_p->port, cb_data_p->port_type);
            ret=TRUE;
        }
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_IF_MAU_CHANGED:
        {
            SYS_CALLBACK_MGR_IfMauChanged_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_IfMauChanged_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_IfMauChanged_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_IfMauChanged_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->lport);
            ret=TRUE;
        }
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_LEARNING_STATUS_CHANGED:
        {
            SYS_CALLBACK_MGR_PortLearningStatusChanged_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_PortLearningStatusChanged_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_PortLearningStatusChanged_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_PortLearningStatusChanged_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->lport, cb_data_p->learning);
            ret=TRUE;
        }
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD_1ST:
        {
            SYS_CALLBACK_MGR_TrunkMember_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_TrunkMember_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_TrunkMember_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_TrunkMember_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->trunk_ifindex, cb_data_p->member_ifindex);
            ret=TRUE;
        }
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD:
        {
            SYS_CALLBACK_MGR_TrunkMember_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_TrunkMember_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_TrunkMember_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_TrunkMember_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->trunk_ifindex, cb_data_p->member_ifindex);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE:
        {
            SYS_CALLBACK_MGR_TrunkMember_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_TrunkMember_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_TrunkMember_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_TrunkMember_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->trunk_ifindex, cb_data_p->member_ifindex);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE_LST:
        {
            SYS_CALLBACK_MGR_TrunkMember_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_TrunkMember_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_TrunkMember_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_TrunkMember_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->trunk_ifindex, cb_data_p->member_ifindex);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_LINK_UP:
        {
            SYS_CALLBACK_MGR_LPort_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_LPort_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_LPort_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_LPort_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->ifindex);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_LINK_UP:
        {
            SYS_CALLBACK_MGR_UPort_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_UPort_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_UPort_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_UPort_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->unit, cb_data_p->port);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_FAST_LINK_UP:
        {
            SYS_CALLBACK_MGR_UPort_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_UPort_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_UPort_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_UPort_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->unit, cb_data_p->port);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_LINK_DOWN:
        {
            SYS_CALLBACK_MGR_LPort_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_LPort_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_LPort_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_LPort_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->ifindex);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_LINK_DOWN:
        {
            SYS_CALLBACK_MGR_UPort_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_UPort_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_UPort_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_UPort_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->unit, cb_data_p->port);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_FAST_LINK_DOWN:
        {
            SYS_CALLBACK_MGR_UPort_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_UPort_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_UPort_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_UPort_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->unit, cb_data_p->port);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_OPER_UP:
        {
            SYS_CALLBACK_MGR_LPort_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_LPort_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_LPort_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_LPort_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->ifindex);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_NOT_OPER_UP:
        {
            SYS_CALLBACK_MGR_LPort_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_LPort_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_LPort_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_LPort_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->ifindex);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_PORT_OPER_UP:
        {
            SYS_CALLBACK_MGR_TrunkMember_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_TrunkMember_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_TrunkMember_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_TrunkMember_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->trunk_ifindex, cb_data_p->member_ifindex);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_PORT_NOT_OPER_UP:
        {
            SYS_CALLBACK_MGR_TrunkMember_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_TrunkMember_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_TrunkMember_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_TrunkMember_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->trunk_ifindex, cb_data_p->member_ifindex);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ACTIVE:
        {
            SYS_CALLBACK_MGR_TrunkMember_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_TrunkMember_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_TrunkMember_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_TrunkMember_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->trunk_ifindex, cb_data_p->member_ifindex);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_INACTIVE:
        {
            SYS_CALLBACK_MGR_TrunkMember_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_TrunkMember_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_TrunkMember_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_TrunkMember_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->trunk_ifindex, cb_data_p->member_ifindex);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_ADMIN_ENABLE:
        {
            SYS_CALLBACK_MGR_LPort_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_LPort_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_LPort_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_LPort_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->ifindex);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_ADMIN_ENABLE:
        {
            SYS_CALLBACK_MGR_UPort_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_UPort_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_UPort_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_UPort_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->unit, cb_data_p->port);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_ADMIN_DISABLE:
        {
            SYS_CALLBACK_MGR_LPort_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_LPort_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_LPort_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_LPort_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->ifindex);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_ADMIN_DISABLE_BEFORE:
        {
            SYS_CALLBACK_MGR_LPort_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_LPort_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_LPort_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_LPort_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->ifindex);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_ADMIN_DISABLE:
        {
            SYS_CALLBACK_MGR_UPort_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_UPort_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_UPort_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_UPort_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->unit, cb_data_p->port);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_STATUS_CHANGED_PASSIVELY:
        {
            SYS_CALLBACK_MGR_LPortStatus_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_LPortStatus_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_LPortStatus_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_LPortStatus_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->ifindex, cb_data_p->status_bool, cb_data_p->status_u32);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_SPEED_DUPLEX:
        {
            SYS_CALLBACK_MGR_LPortSpeedDuplex_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_LPortSpeedDuplex_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_LPortSpeedDuplex_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_LPortSpeedDuplex_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->ifindex, cb_data_p->speed_duplex);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_SPEED_DUPLEX:
        {
            SYS_CALLBACK_MGR_UPortSpeedDuplex_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_UPortSpeedDuplex_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_UPortSpeedDuplex_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_UPortSpeedDuplex_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->unit,cb_data_p->port, cb_data_p->speed_duplex);
            ret=TRUE;
        }
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_LACP_EFFECTIVE_OPER_STATUS_CHANGED:
        {
            SYS_CALLBACK_MGR_uPortLacpEffectiveOperStatus_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_uPortLacpEffectiveOperStatus_CBFun_T  cb_fun;
            cb_data_p = (SYS_CALLBACK_MGR_uPortLacpEffectiveOperStatus_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_uPortLacpEffectiveOperStatus_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->unit,cb_data_p->port, cb_data_p->pre_status, cb_data_p->current_status);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_DOT1X_EFFECTIVE_OPER_STATUS_CHANGED:
        {
            SYS_CALLBACK_MGR_uPortLacpEffectiveOperStatus_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_uPortLacpEffectiveOperStatus_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_uPortLacpEffectiveOperStatus_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_uPortLacpEffectiveOperStatus_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->unit,cb_data_p->port, cb_data_p->pre_status, cb_data_p->current_status);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_EFFECTIVE_OPER_STATUS_CHANGED:
        {
            SYS_CALLBACK_MGR_PortEffectiveOperStatus_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_PortEffectiveOperStatus_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_PortEffectiveOperStatus_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_PortEffectiveOperStatus_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->ifindex, cb_data_p->pre_status, cb_data_p->current_status, cb_data_p->level);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_FORWARDING_UPORT_ADD_TO_TRUNK:
        {
            SYS_CALLBACK_MGR_TrunkMember_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_TrunkMember_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_TrunkMember_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_TrunkMember_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->trunk_ifindex, cb_data_p->member_ifindex);
            ret=TRUE;
        }
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_FORWARDING_TRUNK_MEMBER_DELETE:
        {
            SYS_CALLBACK_MGR_TrunkMember_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_TrunkMember_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_TrunkMember_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_TrunkMember_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->trunk_ifindex, cb_data_p->member_ifindex);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_FORWARDING_TRUNK_MEMBER_TO_NON_FORWARDING:
        {
            SYS_CALLBACK_MGR_TrunkMember_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_TrunkMember_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_TrunkMember_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_TrunkMember_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->trunk_ifindex, cb_data_p->member_ifindex);
            ret=TRUE;
        }

        /* SWDRV */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_LINK_UP:
        {
            SYS_CALLBACK_MGR_SWDRV_PortLinkUp_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_SWDRV_PortLinkUp_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_SWDRV_PortLinkUp_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_SWDRV_PortLinkUp_CBFun_T )callback_fun;
            (*cb_fun)(cb_data_p->unit, cb_data_p->port);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_LINK_DOWN:
        {
            SYS_CALLBACK_MGR_SWDRV_PortLinkDown_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_SWDRV_PortLinkDown_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_SWDRV_PortLinkDown_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_SWDRV_PortLinkDown_CBFun_T )callback_fun;
            (*cb_fun)(cb_data_p->unit, cb_data_p->port);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_CRAFT_PORT_LINK_UP:
        {
            SYS_CALLBACK_MGR_SWDRV_CraftPortLinkUp_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_SWDRV_CraftPortLinkUp_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_SWDRV_CraftPortLinkUp_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_SWDRV_CraftPortLinkUp_CBFun_T )callback_fun;
            (*cb_fun)(cb_data_p->unit);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_CRAFT_PORT_LINK_DOWN:
        {
            SYS_CALLBACK_MGR_SWDRV_CraftPortLinkDown_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_SWDRV_CraftPortLinkDown_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_SWDRV_CraftPortLinkDown_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_SWDRV_CraftPortLinkDown_CBFun_T )callback_fun;
            (*cb_fun)(cb_data_p->unit);
            ret = TRUE;
        }
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_TYPE_CHANGED:
        {
            SYS_CALLBACK_MGR_SWDRV_PortTypeChanged_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_SWDRV_PortTypeChanged_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_SWDRV_PortTypeChanged_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_SWDRV_PortTypeChanged_CBFun_T )callback_fun;
            (*cb_fun)(cb_data_p->unit, cb_data_p->port, cb_data_p->module_id, cb_data_p->port_type);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_SPEED_DUPLEX:
        {
            SYS_CALLBACK_MGR_SWDRV_PortSpeedDuplex_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_SWDRV_PortSpeedDuplex_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_SWDRV_PortSpeedDuplex_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_SWDRV_PortSpeedDuplex_CBFun_T )callback_fun;
            (*cb_fun)(cb_data_p->unit, cb_data_p->port, cb_data_p->speed_duplex);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_FLOW_CTRL:
        {
            SYS_CALLBACK_MGR_SWDRV_PortFlowCtrl_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_SWDRV_PortFlowCtrl_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_SWDRV_PortFlowCtrl_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_SWDRV_PortFlowCtrl_CBFun_T )callback_fun;
            (*cb_fun)(cb_data_p->unit, cb_data_p->port, cb_data_p->flow_ctrl);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_HOT_SWAP_INSERT:
        {
            SYS_CALLBACK_MGR_SWDRV_HotSwapInsert_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_SWDRV_HotSwapInsert_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_SWDRV_HotSwapInsert_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_SWDRV_HotSwapInsert_CBFun_T )callback_fun;
            (*cb_fun)(cb_data_p->unit, cb_data_p->port);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_HOT_SWAP_REMOVE:
        {
            SYS_CALLBACK_MGR_SWDRV_HotSwapRemove_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_SWDRV_HotSwapRemove_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_SWDRV_HotSwapRemove_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_SWDRV_HotSwapRemove_CBFun_T )callback_fun;
            (*cb_fun)(cb_data_p->unit, cb_data_p->port);
            ret = TRUE;
        }
            break;
#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_SFP_PRESENT:
        {
            SYS_CALLBACK_MGR_SWDRV_PortSfpPresent_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_SWDRV_PortSfpPresent_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_SWDRV_PortSfpPresent_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_SWDRV_PortSfpPresent_CBFun_T )callback_fun;
            (*cb_fun)(cb_data_p->unit, cb_data_p->port, cb_data_p->is_present);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_SFP_INFO:
        {
            SYS_CALLBACK_MGR_SWDRV_PortSfpInfo_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_SWDRV_PortSfpInfo_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_SWDRV_PortSfpInfo_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_SWDRV_PortSfpInfo_CBFun_T)callback_fun;
            (*cb_fun)(cb_data_p->unit, cb_data_p->sfp_index, &(cb_data_p->sfp_info));
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_SFP_DDM_INFO:
        {
            SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfo_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfo_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfo_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfo_CBFun_T)callback_fun;
            (*cb_fun)(cb_data_p->unit, cb_data_p->sfp_index, &(cb_data_p->sfp_ddm_info));
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_SFP_DDM_INFO_MEASURED:
        {
            SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfoMeasured_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfoMeasured_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfoMeasured_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfoMeasured_CBFun_T)callback_fun;
            (*cb_fun)(cb_data_p->unit, cb_data_p->sfp_index, &(cb_data_p->sfp_ddm_info_measured));
            ret = TRUE;
        }
            break;
#endif /* #if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE) */

#if (SYS_CPNT_CFM == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_CFM_DEFECT_NOTIFY:
        {
            SYS_CALLBACK_MGR_CFM_DefectNotify_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_CFM_DefectNotify_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_CFM_DefectNotify_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_CFM_DefectNotify_CBFun_T )callback_fun;
            (*cb_fun)(cb_data_p->type, cb_data_p->mep_id, cb_data_p->lport, cb_data_p->level, cb_data_p->vid, cb_data_p->defected);
            ret = TRUE;
        }
            break;
#endif

        /* IGMPSNP */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_IGMPSNP_STATUS_CHANGED:
        {
            SYS_CALLBACK_MGR_IgmpSnoopStatusChanged_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_IgmpSnoopStatusChanged_CBData_T *cb_data_p;

            cb_data_p = (SYS_CALLBACK_MGR_IgmpSnoopStatusChanged_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_IgmpSnoopStatusChanged_CBFun_T)callback_fun;

            (*cb_fun)(cb_data_p->igmpsnp_status);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ROUTER_PORT_ADD:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ROUTER_PORT_DELETE:
        {
            SYS_CALLBACK_MGR_IgmpSnoopRouterPortChanged_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_IgmpSnoopRouterPortChanged_CBData_T *cb_data_p;

            cb_data_p = (SYS_CALLBACK_MGR_IgmpSnoopRouterPortChanged_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_IgmpSnoopRouterPortChanged_CBFun_T)callback_fun;

            (*cb_fun)(cb_data_p->vid_ifidx, cb_data_p->lport_ifidx);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_GROUP_MEMBER_ADD:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_GROUP_MEMBER_DELETE:
        {
            SYS_CALLBACK_MGR_IgmpSnoopGroupMemberChanged_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_IgmpSnoopGroupMemberChanged_CBData_T *cb_data_p;

            cb_data_p = (SYS_CALLBACK_MGR_IgmpSnoopGroupMemberChanged_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_IgmpSnoopGroupMemberChanged_CBFun_T)callback_fun;

            (*cb_fun)(cb_data_p->vid_ifidx, cb_data_p->mip, cb_data_p->lport_ifidx);
            ret=TRUE;
        }
            break;

        /* SYSMGMT */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POWER_STATUS_CHANGED :
        {
            SYS_CALLBACK_MGR_PowerStatusChanged_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_PowerStatusChanged_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_PowerStatusChanged_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_PowerStatusChanged_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->unit,cb_data_p->power, cb_data_p->status);
            ret=TRUE;
        }
            break;
#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_FAN_STATUS_CHANGED :
        {
            SYS_CALLBACK_MGR_FanStatusChanged_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_FanStatusChanged_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_FanStatusChanged_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_FanStatusChanged_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->unit,cb_data_p->fan, cb_data_p->status);
            ret=TRUE;
        }
            break;
#endif
#if (SYS_CPNT_THERMAL_DETECT == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_THERMAL_STATUS_CHANGED :
        {
            SYS_CALLBACK_MGR_ThermalStatusChanged_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_ThermalStatusChanged_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_ThermalStatusChanged_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_ThermalStatusChanged_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->unit,cb_data_p->thermal_idx, cb_data_p->temperature);
            ret=TRUE;
        }
            break;
#endif
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_XFP_MODULE_STATUS_CHANGED :
        {
            SYS_CALLBACK_MGR_XFPModuleStatusChanged_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_XFPModuleStatusChanged_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_XFPModuleStatusChanged_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_XFPModuleStatusChanged_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->unit,cb_data_p->port, cb_data_p->status);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_ALARM_INPUT_STATUS_CHANGED:
        {
            SYS_CALLBACK_MGR_SYSDRV_AlarmInputStatusChanged_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_AlarmInputStatusChanged_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_SYSDRV_AlarmInputStatusChanged_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_AlarmInputStatusChanged_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->unit,cb_data_p->status);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_MAJOR_ALARM_OUTPUT_STATUS_CHANGED:
        {
            SYS_CALLBACK_MGR_SYSDRV_AlarmOutputStatusChanged_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_AlarmOutputStatusChanged_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_SYSDRV_AlarmOutputStatusChanged_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_AlarmOutputStatusChanged_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->unit,cb_data_p->status);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_MINOR_ALARM_OUTPUT_STATUS_CHANGED:
        {
            SYS_CALLBACK_MGR_SYSDRV_AlarmOutputStatusChanged_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_AlarmOutputStatusChanged_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_SYSDRV_AlarmOutputStatusChanged_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_AlarmOutputStatusChanged_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->unit,cb_data_p->status);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_POWER_STATUS_CHANGED:
        {
            SYS_CALLBACK_MGR_SYSDRV_PowerStatusChanged_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_PowerStatusChanged_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_SYSDRV_PowerStatusChanged_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_PowerStatusChanged_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->unit,cb_data_p->power, cb_data_p->status);
            ret=TRUE;
        }
            break;

#if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_POWER_TYPE_CHANGED:
        {
            SYS_CALLBACK_MGR_SYSDRV_PowerTypeChanged_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_PowerTypeChanged_CBData_T     *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_SYSDRV_PowerTypeChanged_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_PowerTypeChanged_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->unit,cb_data_p->power, cb_data_p->type);
            ret=TRUE;
        }
            break;

#endif

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_FAN_STATUS_CHANGED :
        {
            SYS_CALLBACK_MGR_SYSDRV_FanStatusChanged_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_FanStatusChanged_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_SYSDRV_FanStatusChanged_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_FanStatusChanged_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->unit,cb_data_p->fan, cb_data_p->status);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_FAN_SPEED_CHANGED :
        {
            SYS_CALLBACK_MGR_SYSDRV_FanSpeedChanged_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_FanSpeedChanged_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_SYSDRV_FanSpeedChanged_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_FanSpeedChanged_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->unit,cb_data_p->fan, cb_data_p->speed);
            ret=TRUE;
        }
            break;
#endif
#if (SYS_CPNT_THERMAL_DETECT == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_THERMAL_STATUS_CHANGED :
        {
            SYS_CALLBACK_MGR_SYSDRV_ThermalStatusChanged_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_ThermalStatusChanged_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_SYSDRV_ThermalStatusChanged_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_ThermalStatusChanged_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->unit,cb_data_p->thermal_idx, cb_data_p->temperature);
            ret=TRUE;
        }
            break;
#endif
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_XFP_MODULE_STATUS_CHANGED :
        {
            SYS_CALLBACK_MGR_SYSDRV_XFPModuleStatusChanged_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_XFPModuleStatusChanged_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_SYSDRV_XFPModuleStatusChanged_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_XFPModuleStatusChanged_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->unit,cb_data_p->port, cb_data_p->status);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_XENPAK_STATUS_CHANGED:
        {
            SYS_CALLBACK_MGR_SYSDRV_XenpakStatusChanged_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_XenpakStatusChanged_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_SYSDRV_XenpakStatusChanged_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_XenpakStatusChanged_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->xenpak_type);
            ret=TRUE;
        }
            break;

        /* TRK */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ADD_STATIC_TRUNK_MEMBER:
        {
            SYS_CALLBACK_MGR_AddStaticTrunkMember_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_AddStaticTrunkMember_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_AddStaticTrunkMember_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_AddStaticTrunkMember_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->trunk_ifindex, cb_data_p->tm_ifindex);
            ret = TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_DEL_STATIC_TRUNK_MEMBER:
        {
            SYS_CALLBACK_MGR_DelStaticTrunkMember_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_DelStaticTrunkMember_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_DelStaticTrunkMember_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_DelStaticTrunkMember_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->trunk_ifindex, cb_data_p->tm_ifindex);
            ret = TRUE;
        }
            break;

        /* LAN */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_L2MUX_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LACP_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_OAM_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_OAM_LBK_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LOOPBACK_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_DOT1X_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_STA_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_GVRP_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LLDP_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_SFLOW_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LBD_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_UDLD_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_ESMC_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_MLAG_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_AUTHENTICATION_DISPATCH:
        {
            SYS_CALLBACK_MGR_LanReceivePacket_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_LanReceivePacket_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_LanReceivePacket_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_LanReceivePacket_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(L_IPCMEM_GetPtr(cb_data_p->mref_handle_offset),cb_data_p->dst_mac,
                      cb_data_p->src_mac,cb_data_p->tag_info,cb_data_p->type,
                      cb_data_p->pkt_length,cb_data_p->src_unit,cb_data_p->src_port,cb_data_p->packet_class);
            ret = TRUE;
        }
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_AUTHENTICATE_PACKET:
        {
            SYS_CALLBACK_MGR_AuthenticatePacket_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_AuthenticatePacket_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_LanReceivePacket_CBFun_T    lan_cb_fun;
            SYS_CALLBACK_MGR_LanReceivePacket_CBData_T   *lan_cbdata_msg_p;

            cb_data_p = (SYS_CALLBACK_MGR_AuthenticatePacket_CBData_T *)syscallback_msg_p->callback_data;

            if (cb_data_p->flag == 1)
            {
                /* Handler might enqueue this msg first and then dequeue by another thread,
                 * so shouldn't pass syscallback_msg_p->callback_data to handler directly.
                 * Allocate a buffer for it and free it when handler sent this packet
                 * to next handler by calling SYS_CALLBACK_MGR_AuthenticatePacket_AsyncReturn
                 */
                cb_data_p = L_MM_Malloc(sizeof(SYS_CALLBACK_MGR_AuthenticatePacket_CBData_T),
                    L_MM_USER_ID2(SYS_MODULE_SYS_CALLBACK, EXT_TRACE_ID_AUTHENTICATEPACKET_ASYNCRETURN));
                if (cb_data_p == NULL)
                {
                    ret = FALSE;
                    break;
                }

                memcpy(cb_data_p, syscallback_msg_p->callback_data, sizeof(SYS_CALLBACK_MGR_AuthenticatePacket_CBData_T));
            }

                cb_fun = callback_fun;
                lan_cbdata_msg_p = &cb_data_p->lan_cbdata;

                (*cb_fun)(
                    L_IPCMEM_GetPtr(lan_cbdata_msg_p->mref_handle_offset),
                    lan_cbdata_msg_p->dst_mac,
                    lan_cbdata_msg_p->src_mac,
                    lan_cbdata_msg_p->tag_info,
                    lan_cbdata_msg_p->type,
                    lan_cbdata_msg_p->pkt_length,
                    lan_cbdata_msg_p->src_unit,
                    lan_cbdata_msg_p->src_port,
                    cb_data_p->auth_result,
                    cb_data_p);

            ret = TRUE;
        }
            break;

        /* IML */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_IP_PACKET:
        {
            SYS_CALLBACK_MGR_LanReceivePacket_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_LanReceivePacket_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_LanReceivePacket_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_LanReceivePacket_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(L_IPCMEM_GetPtr(cb_data_p->mref_handle_offset),cb_data_p->dst_mac,
                      cb_data_p->src_mac,cb_data_p->tag_info,cb_data_p->type,
                      cb_data_p->pkt_length,cb_data_p->src_unit,cb_data_p->src_port,cb_data_p->packet_class);
            ret = TRUE;
        }
            break;

        /* ISC */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_STKTPLG_PACKET:
        {
            SYS_CALLBACK_MGR_IscReceiveStktplgPacket_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_IscReceiveStktplgPacket_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_IscReceiveStktplgPacket_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_IscReceiveStktplgPacket_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(&(cb_data_p->isc_key),
                      L_IPCMEM_GetPtr(cb_data_p->mref_handle_offset),
                      cb_data_p->rx_port);
            ret = TRUE;
        }
            break;

        /* L2_MUX */
#if 0
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_GVRP_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LLDP_PACKET:
#endif
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_IGMPSNP_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_CLUSTER_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_CFM_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_ELPS_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_RAPS_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_ERPS_HEALTH_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_PPPOED_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_PTP_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RX_SNOOP_DHCP_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RX_DHCP_PACKET:
        {
            SYS_CALLBACK_MGR_L2muxReceivePacket_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_L2muxReceivePacket_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(L_IPCMEM_GetPtr(cb_data_p->mref_handle_offset),cb_data_p->dst_mac,
                      cb_data_p->src_mac,cb_data_p->tag_info,cb_data_p->type,
                      cb_data_p->pkt_length,cb_data_p->lport);
            ret = TRUE;
        }
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TX_SNOOP_DHCP_PACKET:
        {
            SYS_CALLBACK_MGR_TxSnoopDhcpPacket_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_TxSnoopDhcpPacket_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_TxSnoopDhcpPacket_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_TxSnoopDhcpPacket_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(L_IPCMEM_GetPtr(cb_data_p->mref_handle_offset),
                      cb_data_p->dst_mac,
                      cb_data_p->src_mac,
                      cb_data_p->pkt_len,
                      cb_data_p->egr_vidifindex,
                      cb_data_p->egr_lport,
                      cb_data_p->ing_lport);
            ret = TRUE;
        }
            break;
        /* IML*/
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_BOOTP_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_UDP_HELPER_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_HSRP_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_VRRP_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_ARP_PACKET:
        {
            SYS_CALLBACK_MGR_ImlReceivePacket_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_ImlReceivePacket_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_ImlReceivePacket_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_ImlReceivePacket_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(L_IPCMEM_GetPtr(cb_data_p->mref_handle_offset),
                      cb_data_p->packet_length, cb_data_p->ifindex,
                      cb_data_p->dst_mac,cb_data_p->src_mac,
                      cb_data_p->ingress_vid,cb_data_p->src_port);
            ret = TRUE;
        }
            break;
#if(SYS_CPNT_DHCPV6SNP == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_DHCPV6SNP_PACKET:
        {
            SYS_CALLBACK_MGR_Dhcp6snpReceivePacket_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_Dhcp6snpReceivePacket_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_Dhcp6snpReceivePacket_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_Dhcp6snpReceivePacket_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(L_IPCMEM_GetPtr(cb_data_p->mref_handle_offset),
                      cb_data_p->packet_length,
                      cb_data_p->ext_hdr_len,
                      cb_data_p->dst_mac,cb_data_p->src_mac,
                      cb_data_p->ingress_vid,cb_data_p->src_port);
            ret = TRUE;
        }
            break;
#endif
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_MLDSNP_PACKET:
        {
            SYS_CALLBACK_MGR_MldsnpReceivePacket_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_MldsnpReceivePacket_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_MldsnpReceivePacket_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_MldsnpReceivePacket_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(L_IPCMEM_GetPtr(cb_data_p->mref_handle_offset),cb_data_p->dst_mac,
                      cb_data_p->src_mac,cb_data_p->tag_info,cb_data_p->type,
                      cb_data_p->pkt_length, cb_data_p->ext_hdr_len, cb_data_p->lport);
            ret = TRUE;
        }
            break;

#if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_RAGUARD_PACKET:
        {
            SYS_CALLBACK_MGR_RaGuardReceivePacket_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_RaGuardReceivePacket_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_RaGuardReceivePacket_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_RaGuardReceivePacket_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(L_IPCMEM_GetPtr(cb_data_p->mref_handle_offset),cb_data_p->dst_mac,
                      cb_data_p->src_mac, cb_data_p->ing_vid, cb_data_p->ing_cos,
                      cb_data_p->pkt_type, cb_data_p->pkt_length, cb_data_p->src_lport);
            ret = TRUE;
        }
            break;
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE) */

        /* STKTPLG */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_STACK_STATE:
        {
            SYS_CALLBACK_MGR_StackState_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_StackState_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_StackState_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_StackState_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->msg);
            ret=TRUE;
        }
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MODULE_STATE_CHANGED:
        {
            SYS_CALLBACK_MGR_ModuleStateChanged_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_ModuleStateChanged_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_ModuleStateChanged_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_ModuleStateChanged_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->unit_id);
            ret=TRUE;
        }

            break;

        /* STKCTRL */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SAVING_CONFIG_STATUS:
        {
            SYS_CALLBACK_MGR_SavingConfigStatus_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_SavingConfigStatus_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_SavingConfigStatus_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_SavingConfigStatus_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->status);
            ret=TRUE;
        }
            break;


        /* NETCFG */
        /* Triggered by IPCFG
         */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_CREATED:
        {
            SYS_CALLBACK_MGR_RifCreated_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_RifCreated_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_RifCreated_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_RifCreated_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->ifindex, &(cb_data_p->addr));
            ret=TRUE;
        }
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_ACTIVE:
        {
            SYS_CALLBACK_MGR_RifActive_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_RifActive_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_RifActive_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_RifActive_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->ifindex, &(cb_data_p->addr));
            ret=TRUE;
        }
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_DOWN:
        {
            SYS_CALLBACK_MGR_RifDown_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_RifDown_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_RifDown_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_RifDown_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->ifindex, &(cb_data_p->addr));
            ret=TRUE;
        }
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_DESTROYED:
        {
            SYS_CALLBACK_MGR_RifDestroyed_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_RifDestroyed_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_RifDestroyed_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_RifDestroyed_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->ifindex, &(cb_data_p->addr));
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_IPCFG_NSM_ROUTE_CHANGE:
        {
            SYS_CALLBACK_MGR_NsmRouteChange_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_IPCFG_NsmRouteChange_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_NsmRouteChange_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_IPCFG_NsmRouteChange_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->address_family);
            ret=TRUE;
        }
            break;
#if (SYS_CPNT_DHCPV6 == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_IPV6_ADDRAUTOCONFIG:
        {
            SYS_CALLBACK_MGR_NETCFG_IPv6AddrAutoConfigCBFun_T  cb_fun;
            SYS_CALLBACK_MGR_NETCFG_ifindex_bool_v_CBData_T*cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_NETCFG_IPv6AddrAutoConfigCBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_NETCFG_ifindex_bool_v_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->ifindex, cb_data_p->bool_v);
            ret=TRUE;
        }
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_L3IF_CREATE:
        {
            SYS_CALLBACK_MGR_NETCFG_L3IfCreateCBFun_T  cb_fun;
            SYS_CALLBACK_MGR_NETCFG_ifindex_bool_v_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_NETCFG_L3IfCreateCBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_NETCFG_ifindex_bool_v_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->ifindex, cb_data_p->bool_v);
            ret=TRUE;
        }
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_L3IF_DESTROY:
        {
            SYS_CALLBACK_MGR_NETCFG_L3IfDestroyCBFun_T  cb_fun;
            SYS_CALLBACK_MGR_NETCFG_ifindex_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_NETCFG_L3IfDestroyCBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_NETCFG_ifindex_CBData_T *)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->ifindex);
            ret=TRUE;
        }
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_L3IF_OPER_STATUS_UP:
        {
            SYS_CALLBACK_MGR_NETCFG_L3IfOperStatusUpCBFun_T cb_fun;
            SYS_CALLBACK_MGR_NETCFG_ifindex_bool_v_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_NETCFG_L3IfOperStatusUpCBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_NETCFG_ifindex_bool_v_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->ifindex, cb_data_p->bool_v);
            ret=TRUE;
        }
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_L3IF_OPER_STATUS_DOWN:
        {
            SYS_CALLBACK_MGR_NETCFG_L3IfOperStatusDownCBFun_T  cb_fun;
            SYS_CALLBACK_MGR_NETCFG_ifindex_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_NETCFG_L3IfOperStatusDownCBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_NETCFG_ifindex_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->ifindex);
            ret=TRUE;
        }
            break;

#endif
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PROVISION_COMPLETE:
        {
            SYS_CALLBACK_MGR_ProvisionComplete_CBFun_T  cb_fun;

            cb_fun = (SYS_CALLBACK_MGR_ProvisionComplete_CBFun_T)callback_fun;

            (*cb_fun)();
            ret=TRUE;
        }
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MODULE_PROVISION_COMPLETE:
        {
            SYS_CALLBACK_MGR_ModuleProvisionComplete_CBFun_T  cb_fun;

            cb_fun = (SYS_CALLBACK_MGR_ModuleProvisionComplete_CBFun_T)callback_fun;

            (*cb_fun)();
            ret=TRUE;
        }
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ENTER_TRANSITION_MODE:
        {
            SYS_CALLBACK_MGR_EnterTransitionMode_CBFun_T  cb_fun;

            cb_fun = (SYS_CALLBACK_MGR_EnterTransitionMode_CBFun_T)callback_fun;

            (*cb_fun)();
            ret=TRUE;
        }
            break;

        /* LLDP */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TELEPHONE_DETECT:
        {
            SYS_CALLBACK_MGR_TelephoneDetect_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_TelephoneDetect_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_TelephoneDetect_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_TelephoneDetect_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->lport,
                      cb_data_p->mac_addr,
                      cb_data_p->network_addr_subtype,
                      cb_data_p->network_addr,
                      cb_data_p->network_addr_len,
                      cb_data_p->network_addr_ifindex,
                      cb_data_p->tel_exist);
            ret = TRUE;
        }
            break;

#if (SYS_CPNT_POE == TRUE)
#ifdef SYS_CPNT_POE_PSE_DOT3AT
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_DOT3AT_INFO_RECEIVED:
        {
            SYS_CALLBACK_MGR_Dot3atInfoReceived_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_Dot3atInfoReceived_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_Dot3atInfoReceived_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_Dot3atInfoReceived_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->unit,
                      cb_data_p->port,
                      cb_data_p->power_type,
                      cb_data_p->power_source,
                      cb_data_p->power_priority,
                      cb_data_p->power_value,
                      cb_data_p->requested_power_type,
                      cb_data_p->requested_power_source,
                      cb_data_p->requested_power_priority,
                      cb_data_p->requested_power_value,
                      cb_data_p->acknowledge);
            ret = TRUE;
        }
            break;
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_DOT3AT_INFO_RECEIVED:
        {
            SYS_CALLBACK_MGR_Dot3atInfoReceived_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_Dot3atInfoReceived_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_Dot3atInfoReceived_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_Dot3atInfoReceived_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->unit,
                      cb_data_p->port,
                      cb_data_p->power_type,
                      cb_data_p->power_source,
                      cb_data_p->power_priority,
                      cb_data_p->pd_requested_power,
                      cb_data_p->pse_allocated_power);
            ret = TRUE;
        }
            break;
#endif
#endif
#endif

#if (SYS_CPNT_CN)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_CN_REMOTE_CHANGE:
        {
            SYS_CALLBACK_MGR_CnRemoteChange_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_CnRemoteChange_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_CnRemoteChange_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_CnRemoteChange_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->lport, cb_data_p->neighbor_num,
                cb_data_p->cnpv_indicators, cb_data_p->ready_indicators);
            ret = TRUE;
        }
            break;
#endif /* #if (SYS_CPNT_CN) */

        /* PRIMGMT */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_COS_CHANGED:
        {
            SYS_CALLBACK_MGR_CosChanged_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_CosChanged_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_CosChanged_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_CosChanged_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->lport_ifindex);
            ret = TRUE;
        }
            break;

        /* RADIUS */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RADIUS_PACKET:
        {
            SYS_CALLBACK_MGR_AnnounceRADIUSPacket_CBFun_T  cb_fun;
            SYS_CALLBACK_MGR_AnnounceRADIUSPacket_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_AnnounceRADIUSPacket_CBFun_T) callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_AnnounceRADIUSPacket_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->result,                cb_data_p->data_buf,
                      cb_data_p->data_len,              cb_data_p->src_port,
                      cb_data_p->src_mac,               cb_data_p->src_vid,
                      cb_data_p->authorized_vlan_list,  cb_data_p->authorized_qos_list,
                      cb_data_p->session_timeout,       cb_data_p->server_ip);
            ret = TRUE;
        }
            break;
#if(SYS_CPNT_IGMPAUTH == TRUE)
        case  SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RADIUS_IGMPAUTH_RESULT:
        {
            SYS_CALLBACK_MGR_AnnounceRadiusIGMPAuthResult_CBFun_T cb_fun;
            SYS_CALLBACK_MGR_AnnounceRadiusIGMPAuthResult_CBData_T   *cb_data_p;
            cb_fun    = (SYS_CALLBACK_MGR_AnnounceRadiusIGMPAuthResult_CBFun_T)      callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_AnnounceRadiusIGMPAuthResult_CBData_T *)   syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->result,
                      cb_data_p->auth_port,
                      cb_data_p->ip_address,
                      cb_data_p->auth_mac,
                      cb_data_p->vlan_id,
                      cb_data_p->src_ip,
                      cb_data_p->msg_type);
            ret = TRUE;
        }
            break;
#endif

        /*maggie liu for RADIUS authentication ansync*/
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RADIUS_AUTH_RESULT:
        {
            SYS_CALLBACK_MGR_AnnounceRadiusAuthRes_CBFun_T cb_fun;
            SYS_CALLBACK_MGR_AnnounceRADIUSAuthenRadius_CBData_T   *cb_data_p;

            cb_fun    = (SYS_CALLBACK_MGR_AnnounceRadiusAuthRes_CBFun_T)      callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_AnnounceRADIUSAuthenRadius_CBData_T *)   syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->result, cb_data_p->privilege, cb_data_p->cookie);
            ret = TRUE;
        }
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RADIUS_AUTH_RESULT_2:
        {
            SYS_CALLBACK_MGR_AnnounceRadiusAuthRes_2_CBFun_T cb_fun;
            SYS_CALLBACK_MGR_AnnounceRADIUSAuthenRadius_2_CBData_T   *cb_data_p;

            cb_fun    = (SYS_CALLBACK_MGR_AnnounceRadiusAuthRes_2_CBFun_T)      callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_AnnounceRADIUSAuthenRadius_2_CBData_T *)   syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->result, cb_data_p->privilege, cb_data_p->cookie, cb_data_p->cookie_size);
            ret = TRUE;
        }
            break;


        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RADA_AUTH_RESULT:
        {
            SYS_CALLBACK_MGR_AnnounceRadaAuthRes_CBFun_T cb_fun;
            SYS_CALLBACK_MGR_AnnounceRadaAuthRes_CBData_T   *cb_data_p;

            cb_fun    = (SYS_CALLBACK_MGR_AnnounceRadaAuthRes_CBFun_T)      callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_AnnounceRadaAuthRes_CBData_T *)   syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->lport, cb_data_p->authorized_mac,
                      cb_data_p->identifier, cb_data_p->authorized_result,
                      cb_data_p->authorized_vlan_list, cb_data_p->authorized_qos_list,
                      cb_data_p->session_time, cb_data_p->server_ip);
            ret = TRUE;
        }
            break;

        /* DOT1X */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_DOT1X_AUTH_RESULT:
        {
            SYS_CALLBACK_MGR_AnnounceDot1xAuthRes_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_AnnounceDot1xAuthRes_CBData_T  *cb_data_p;

            cb_fun    = (SYS_CALLBACK_MGR_AnnounceDot1xAuthRes_CBFun_T)     callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_AnnounceDot1xAuthRes_CBData_T *)  syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->lport, cb_data_p->authorized_mac,
                      cb_data_p->eap_identifier, cb_data_p->authorized_result,
                      cb_data_p->authorized_vlan_list, cb_data_p->authorized_qos_list,
                      cb_data_p->session_time, cb_data_p->server_ip);
            ret = TRUE;
        }
            break;

        /* XFER */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_XFER_RESULT:
        {
            SYS_CALLBACK_MGR_AnnounceXferResult_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_AnnounceXferResult_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_AnnounceXferResult_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_AnnounceXferResult_CBFun_T )callback_fun;

            (*cb_fun)(syscallback_msg_p->cookie ,cb_data_p->cookie, cb_data_p->status);
            ret=TRUE;
        }
            break;

        /* HTTP SSHD */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_CLI_XFER_RESULT:
        {
            SYS_CALLBACK_MGR_AnnounceCliXferResult_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_AnnounceCliXferResult_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_AnnounceCliXferResult_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_AnnounceCliXferResult_CBFun_T )callback_fun;

            (*cb_fun)(syscallback_msg_p->cookie ,cb_data_p->cookie, cb_data_p->status);
            ret=TRUE;
        }
            break;

        /* AMTRL3 */
#if (SYS_CPNT_AMTRL3 == TRUE)
#if 0
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_NEXTHOP_STATUS_CHANGE:
        {
            SYS_CALLBACK_MGR_Nexthop_Status_Change_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_Nexthop_Status_Change_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_Nexthop_Status_Change_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_Nexthop_Status_Change_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->action_flags, cb_data_p->fib_id, cb_data_p->status, cb_data_p->ip_addr,
                      cb_data_p->lport_ifindex, cb_data_p->vid_ifindex, cb_data_p->dst_mac);
            ret = TRUE;
        }
            break;
#endif
#endif
#if (SYS_CPNT_DHCP == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_DHCP_RESTART3:
        {
            SYS_CALLBACK_MGR_DHCP_Restart3_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_DHCP_Restart3_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_DHCP_Restart3_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_DHCP_Restart3_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->restart_object);
            ret=TRUE;
        }
        break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_DHCP_SET_IF_ROLE:
        {
            SYS_CALLBACK_MGR_DHCP_SetIfRole_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_DHCP_SetIfRole_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_DHCP_SetIfRole_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_DHCP_SetIfRole_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->vid_ifindex, cb_data_p->role);
            ret=TRUE;
        }

        break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_DHCP_SET_IF_STATUS:
            {
                SYS_CALLBACK_MGR_DHCP_SetIfStatud_CBData_T *cb_data_p;
                SYS_CALLBACK_MGR_DHCP_SetIfStatus_CBFun_T  cb_fun;

                cb_data_p = (SYS_CALLBACK_MGR_DHCP_SetIfStatud_CBData_T *)syscallback_msg_p->callback_data;
                cb_fun = (SYS_CALLBACK_MGR_DHCP_SetIfStatus_CBFun_T )callback_fun;

                (*cb_fun)(cb_data_p->vid_ifindex, cb_data_p->status);
                ret=TRUE;
            }
            break;
#endif
#if (SYS_CPNT_CLI_DYNAMIC_PROVISION_VIA_DHCP == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_CLI_DYNAMIC_PROVISION_VIA_DHCP:
        {
            SYS_CALLBACK_MGR_DHCP_RxOptionConfig_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_DHCP_RxOptionConfig_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_DHCP_RxOptionConfig_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_DHCP_RxOptionConfig_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->option66_length, cb_data_p->option66_data, cb_data_p->option67_length, cb_data_p->option67_data);
            ret=TRUE;
        }
        break;
#endif

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RELOAD_REMAINING_TIME:
        {
            SYS_CALLBACK_MGR_AnnouceReloadRemainTime_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_AnnouceReloadRemainDate_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_AnnouceReloadRemainTime_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_AnnouceReloadRemainDate_CBFun_T)callback_fun;

            (*cb_fun)(cb_data_p->remaining_minutes);
            ret = TRUE;
        }
        break;
#endif /* SYS_CPNT_SYSMGMT_DEFERRED_RELOAD */
#if (SYS_CPNT_IP_TUNNEL == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_IPV6_PACKET:
        {
            SYS_CALLBACK_MGR_AnnouceIpv6Packet_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_AnnouceIpv6Packet_CBFun_T  cb_fun;
            cb_data_p = (SYS_CALLBACK_MGR_AnnouceIpv6Packet_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_AnnouceIpv6Packet_CBFun_T)callback_fun;
            (*cb_fun)(cb_data_p->source_address, cb_data_p->destination_address);
            ret = TRUE;
        }
        break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TUNNEL_NET_ROUTE_HIT_BIT_CHANGE:
        {
            SYS_CALLBACK_MGR_TunnelNetRouteHitBitChange_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_TunnelNetRouteHitBitChange_CBFun_T  cb_fun;
            cb_data_p = (SYS_CALLBACK_MGR_TunnelNetRouteHitBitChange_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_TunnelNetRouteHitBitChange_CBFun_T)callback_fun;
            (*cb_fun)(cb_data_p->fib_id, cb_data_p->dst_addr, cb_data_p->preflen, cb_data_p->unit_id);
            ret = TRUE;
        }
        break;
#endif/*SYS_CPNT_IP_TUNNEL*/

#if (SYS_CPNT_POE == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_DETECTION_STATUS_CHANGE:
        {
            SYS_CALLBACK_MGR_POEDRV_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_POEDRV_PortDetectionStatusChange_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_POEDRV_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_POEDRV_PortDetectionStatusChange_CBFun_T )callback_fun;
            (*cb_fun)(cb_data_p->unit, cb_data_p->port, cb_data_p->value);
            ret = TRUE;
        }
        break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_STATUS_CHANGE:
        {
            SYS_CALLBACK_MGR_POEDRV_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_POEDRV_PortStatusChange_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_POEDRV_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_POEDRV_PortStatusChange_CBFun_T )callback_fun;
            (*cb_fun)(cb_data_p->unit, cb_data_p->port, cb_data_p->value);
            ret = TRUE;
        }
        break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_IS_MAIN_POWER_REACH_MAXIMUM:
        {
            SYS_CALLBACK_MGR_POEDRV_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_POEDRV_IsMainPowerReachMaximum_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_POEDRV_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_POEDRV_IsMainPowerReachMaximum_CBFun_T )callback_fun;
            (*cb_fun)(cb_data_p->unit, cb_data_p->value);
            ret = TRUE;
        }
        break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_OVERLOAD_STATUS_CHANGE:
        {
            SYS_CALLBACK_MGR_POEDRV_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_POEDRV_PortOverloadStatusChange_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_POEDRV_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_POEDRV_PortOverloadStatusChange_CBFun_T )callback_fun;
            (*cb_fun)(cb_data_p->unit, cb_data_p->port, cb_data_p->value);
            ret = TRUE;
        }
        break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_POWER_CONSUMPTION_CHANGE:
        {
            SYS_CALLBACK_MGR_POEDRV_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_POEDRV_PortPowerConsumptionChange_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_POEDRV_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_POEDRV_PortPowerConsumptionChange_CBFun_T )callback_fun;
            (*cb_fun)(cb_data_p->unit, cb_data_p->port, cb_data_p->value);
            ret = TRUE;
        }
        break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_POWER_CLASSIFICATION_CHANGE:
        {
            SYS_CALLBACK_MGR_POEDRV_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_POEDRV_PortPowerClassificationChange_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_POEDRV_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_POEDRV_PortPowerClassificationChange_CBFun_T )callback_fun;
            (*cb_fun)(cb_data_p->unit, cb_data_p->port, cb_data_p->value);
            ret = TRUE;
        }
        break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_MAIN_PSE_CONSUMPTION_CHANGE:
        {
            SYS_CALLBACK_MGR_POEDRV_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_POEDRV_MainPseConsumptionChange_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_POEDRV_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_POEDRV_MainPseConsumptionChange_CBFun_T )callback_fun;
            (*cb_fun)(cb_data_p->unit, cb_data_p->value);
            ret = TRUE;
        }
        break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PSE_OPER_STATUS_CHANGE:
        {
            SYS_CALLBACK_MGR_POEDRV_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_POEDRV_PseOperStatusChange_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_POEDRV_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_POEDRV_PseOperStatusChange_CBFun_T )callback_fun;
            (*cb_fun)(cb_data_p->unit, cb_data_p->value);
            ret = TRUE;
        }
        break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_POWER_DENIED_OCCUR_FRENQUENTLY:
        {
            SYS_CALLBACK_MGR_POEDRV_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_POEDRV_PowerDeniedOccurFrequently_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_POEDRV_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_POEDRV_PowerDeniedOccurFrequently_CBFun_T )callback_fun;
            (*cb_fun)(cb_data_p->unit, cb_data_p->port);
            ret = TRUE;
        }
        break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_FAILURE_STATUS_CHANGE:
        {
            SYS_CALLBACK_MGR_POEDRV_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_POEDRV_PortFailureStatusChange_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_POEDRV_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_POEDRV_PortFailureStatusChange_CBFun_T )callback_fun;
            (*cb_fun)(cb_data_p->unit, cb_data_p->port, cb_data_p->value);
            ret = TRUE;
        }
        break;
#endif

#if (SYS_CPNT_CLUSTER == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_CLUSTER_CHANGEROLE:
        {
            SYS_CALLBACK_MGR_CLUSTER_ChangeRole_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_CLUSTER_ChangeRole_CBFun_T cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_CLUSTER_ChangeRole_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_CLUSTER_ChangeRole_CBFun_T)callback_fun;

            (*cb_fun)(cb_data_p->role);
            ret = TRUE;
        }
        break;
#endif

#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ACL_DELETED:
        {
            SYS_CALLBACK_MGR_AnnounceAclDeleted_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_AnnounceAclDeleted_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_AnnounceAclDeleted_CBData_T*)syscallback_msg_p->callback_data;
            cb_fun    = (SYS_CALLBACK_MGR_AnnounceAclDeleted_CBFun_T)callback_fun;

            (*cb_fun)(cb_data_p->acl_name, cb_data_p->acl_type, cb_data_p->dynamic_port_list);
            ret = TRUE;
        }
        break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POLICY_MAP_DELETED:
        {
            SYS_CALLBACK_MGR_AnnouncePolicyMapDeleted_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_AnnouncePolicyMapDeleted_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_AnnouncePolicyMapDeleted_CBData_T*)syscallback_msg_p->callback_data;
            cb_fun    = (SYS_CALLBACK_MGR_AnnouncePolicyMapDeleted_CBFun_T)callback_fun;

            (*cb_fun)(cb_data_p->policy_map_name, cb_data_p->dynamic_port_list);
            ret = TRUE;
        }
        break;
#endif /* #if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE) */

#if (SYS_CPNT_COS == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_COS_PORT_CONFIG_CHANGED:
        {
            SYS_CALLBACK_MGR_AnnounceCosPortConfigChanged_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_AnnounceCosPortConfigChanged_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_AnnounceCosPortConfigChanged_CBData_T*)syscallback_msg_p->callback_data;
            cb_fun    = (SYS_CALLBACK_MGR_AnnounceCosPortConfigChanged_CBFun_T)callback_fun;

            (*cb_fun)(cb_data_p->l_port, cb_data_p->priority_of_config);
            ret = TRUE;
        }
        break;
#endif /* #if (SYS_CPNT_COS == TRUE) */

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MGMT_IP_FLT_CHANGED:
        {
            SYS_CALLBACK_MGR_AnnounceMgmtIPFltChanged_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_AnnounceMgmtIPFltChanged_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_AnnounceMgmtIPFltChanged_CBData_T*)syscallback_msg_p->callback_data;
            cb_fun    = (SYS_CALLBACK_MGR_AnnounceMgmtIPFltChanged_CBFun_T)callback_fun;

            (*cb_fun)(cb_data_p->mode);
            ret = TRUE;
        }
        break;
#endif /* #if (SYS_CPNT_MGMT_IP_FLT == TRUE)*/

        /* CMGR */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SET_PORT_STATUS:
        {
            SYS_CALLBACK_MGR_LPortStatus_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_LPortStatus_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_LPortStatus_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_LPortStatus_CBFun_T )callback_fun;

            (*cb_fun)(cb_data_p->ifindex, cb_data_p->status_bool, cb_data_p->status_u32);
            ret=TRUE;
        }
            break;

#if(SYS_CPNT_DCBX == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ETS_CFG_CHANGED:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PFC_CFG_CHANGED:
        {
            SYS_CALLBACK_MGR_EtsPfcCfgChanged_CBFun_T   cb_fun;
            SYS_CALLBACK_MGR_EtsPfcCfgChanged_CBData_T  *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_EtsPfcCfgChanged_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_EtsPfcCfgChanged_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->lport);
            ret = TRUE;
        }
            break;

        /* DCBX ETS */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_ETS_TLV:
        {
            SYS_CALLBACK_MGR_EtsReceived_CBFun_T    cb_fun;
            SYS_CALLBACK_MGR_DcbxEtsTlvReceived_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_EtsReceived_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_DcbxEtsTlvReceived_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->lport,
                      cb_data_p->is_delete,
                      cb_data_p->rem_recommend_rcvd,
                      cb_data_p->rem_willing,
                      cb_data_p->rem_cbs,
                      cb_data_p->rem_max_tc,
                      cb_data_p->rem_config_pri_assign_table,
                      cb_data_p->rem_config_tc_bandwidth_table,
                      cb_data_p->rem_config_tsa_assign_table,
                      cb_data_p->rem_recommend_pri_assign_table,
                      cb_data_p->rem_recommend_tc_bandwidth_table,
                      cb_data_p->rem_recommend_tsa_assign_table);
            ret = TRUE;
        }
            break;

        /* DCBX PFC */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_PFC_TLV:
        {
            SYS_CALLBACK_MGR_PfcReceived_CBFun_T    cb_fun;
            SYS_CALLBACK_MGR_DcbxPfcTlvReceived_CBData_T *cb_data_p;

            cb_fun = (SYS_CALLBACK_MGR_PfcReceived_CBFun_T)callback_fun;
            cb_data_p = (SYS_CALLBACK_MGR_DcbxPfcTlvReceived_CBData_T*)syscallback_msg_p->callback_data;

            (*cb_fun)(cb_data_p->lport,
                      cb_data_p->is_delete,
                      cb_data_p->rem_mac,
                      cb_data_p->rem_willing,
                      cb_data_p->rem_mbc,
                      cb_data_p->rem_pfc_cap,
                      cb_data_p->rem_pfc_enable);
            ret = TRUE;
        }
            break;
#endif

#if (SYS_CPNT_PBR == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_HOST_ROUTE_CHANGED:
        {
            SYS_CALLBACK_MGR_HostRouteChanged_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_HostRouteChanged_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_HostRouteChanged_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_HostRouteChanged_CBFun_T)callback_fun;

            (*cb_fun)(&cb_data_p->addr, cb_data_p->is_unresolved);
            ret = TRUE;
        }
        break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ACL_CHANGED:
        {
            SYS_CALLBACK_MGR_AclChanged_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_AclChanged_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_AclChanged_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_AclChanged_CBFun_T)callback_fun;

            (*cb_fun)(cb_data_p->acl_index, cb_data_p->acl_name, cb_data_p->type);
            ret = TRUE;
        }
        break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ROUTEMAP_CHANGED:
        {
            SYS_CALLBACK_MGR_RoutemapChanged_CBData_T *cb_data_p;
            SYS_CALLBACK_MGR_RouteMapChanged_CBFun_T  cb_fun;

            cb_data_p = (SYS_CALLBACK_MGR_RoutemapChanged_CBData_T *)syscallback_msg_p->callback_data;
            cb_fun = (SYS_CALLBACK_MGR_RouteMapChanged_CBFun_T)callback_fun;

            (*cb_fun)(cb_data_p->rmap_name, cb_data_p->seq_num, cb_data_p->is_deleted);
            ret = TRUE;
        }
        break;
#endif

        default:
            ret = FALSE;
    }

    return ret;
}

/********************************
 *  callback fail related APIs  *
 ********************************
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_GetAndClearFailInfo
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get and clear the information about failed sender.
 * INPUT    :   cscgroup_mgr_msgqkey   -  The mgr msgq key of the csc group who
 *                                        retrieves its callback information
 * OUTPUT   :   fail_entry_p - The buffer to store the retrieval information.
 * RETURN   :   TRUE  - Successful.
 *              FALSE - Failed.
 * NOTES    :
 *            1.The csc who fails to deliver the callback message will be kept
 *              in fail_entry_p->csc_list. The repeated failure from same csc
 *              will occupy only one entry in fail_entry_p->csc_list.
 *            2.fail_entry_p->csc_list_counter indicate the number of csc contained
 *              in csc_list. Note that if fail_entry_p->csc_list is too small to
 *              keep all cscs who fails to deliver. fail_entry_p->csc_list_counter
 *              will be set as SYS_CALLBACK_MGR_CSC_LIST_OVERFLOW. Caller should
 *              assume that all cscs that will send callback have ever failed to
 *              delivery at least once.
 * ------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_GetAndClearFailInfo(UI32_T cscgroup_mgr_msgqkey, SYS_CALLBACK_OM_FailEntry_T *fail_entry_p)
{
    return SYS_CALLBACK_OM_GetAndClearFailInfo(cscgroup_mgr_msgqkey, fail_entry_p);
}

#if (SYS_CPNT_OVSVTEP == TRUE)
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_OvsMacUpdateCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When dynamic mac add/remove, AMTR will notify other CSC groups by this function.
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      dest_msgq_key    -- key of the destination message queue
 *      ifindex          -- port which the mac is learnt now
 *      vid              -- which vlan id
 *      mac_p            -- mac address
 *      is_add           -- dynamic mac add or remove
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_OvsMacUpdateCallback(UI32_T src_csc_id, UI32_T  dest_msgq_key, UI32_T ifindex, UI16_T vid, UI8_T *mac_p, BOOL_T is_add)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_MacNotify_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_MacNotify_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_MacNotify_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_MacNotify_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_OVS_MAC_UPDATE;
    cbdata_msg_p->ifindex = ifindex;
    cbdata_msg_p->vid     = vid;
    memcpy(cbdata_msg_p->mac, mac_p, sizeof(cbdata_msg_p->mac));
    cbdata_msg_p->is_add = is_add;

    return SYS_CALLBACK_MGR_SendMsgWithMsgQKey(src_csc_id, dest_msgq_key, sysfun_msg_p, TRUE);
}
#endif

#if (SYS_CPNT_MLAG == TRUE)
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_MlagMacUpdateCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When dynamic mac add/remove, AMTR will notify other CSC groups by this function.
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      dest_msgq_key    -- key of the destination message queue
 *      ifindex          -- port which the mac is learnt now
 *      vid              -- which vlan id
 *      mac_p            -- mac address
 *      is_add           -- dynamic mac add or remove
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_MlagMacUpdateCallback(UI32_T src_csc_id, UI32_T  dest_msgq_key, UI32_T ifindex, UI16_T vid, UI8_T *mac_p, BOOL_T is_add)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_MacNotify_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_MacNotify_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_MacNotify_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_MacNotify_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_MLAG_MAC_UPDATE;
    cbdata_msg_p->ifindex = ifindex;
    cbdata_msg_p->vid     = vid;
    memcpy(cbdata_msg_p->mac, mac_p, sizeof(cbdata_msg_p->mac));
    cbdata_msg_p->is_add = is_add;

    return SYS_CALLBACK_MGR_SendMsgWithMsgQKey(src_csc_id, dest_msgq_key, sysfun_msg_p, TRUE);
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveMlagPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When LAN received packet for MLAG, it will notify MLAG with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      tag_info       --- packet tag info
 *      type           --- packet type
 *      pkt_length     --- packet length
 *      src_unit       --- source unit
 *      src_port       --- source port
 *      packet_class   --- packet classified by LAN
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveMlagPacketCallback(UI32_T             src_csc_id,
                                                  L_MM_Mref_Handle_T *mref_handle_p,
                                                  UI8_T              dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                  UI8_T              src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                  UI16_T             tag_info,
                                                  UI16_T             type,
                                                  UI32_T             pkt_length,
                                                  UI32_T             src_unit,
                                                  UI32_T             src_port,
                                                  UI32_T             packet_class)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_LanReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_MLAG_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LanReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LanReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_MLAG_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->tag_info           = tag_info;
    cbdata_msg_p->type               = type;
    cbdata_msg_p->pkt_length         = pkt_length;
    cbdata_msg_p->src_unit           = src_unit;
    cbdata_msg_p->src_port           = src_port;
    cbdata_msg_p->packet_class       = packet_class;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count > 0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        }
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
#endif /* #if (SYS_CPNT_MLAG == TRUE) */

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_MacAddrUpdateCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When mac add/remove, AMTR will notify other CSC groups by this function.
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      ifindex          -- port which the mac is learnt now
 *      vid              -- which vlan id
 *      mac_p            -- mac address
 *      is_add           -- mac add or remove
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_MacAddrUpdateCallback(UI32_T src_csc_id, UI32_T ifindex, UI16_T vid, UI8_T *mac_p, BOOL_T is_add)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T *syscb_msg_p;
    SYS_CALLBACK_MGR_MacNotify_CBData_T *cbdata_msg_p;
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_MacNotify_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_MacNotify_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_MacNotify_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_ADDR_UPDATE;
    cbdata_msg_p->ifindex          = ifindex;
    cbdata_msg_p->vid              = vid;
    memcpy(cbdata_msg_p->mac, mac_p, sizeof(cbdata_msg_p->mac));
    cbdata_msg_p->is_add = is_add;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* LOCAL SUBPROGRAM BODIES
 */
#ifndef UNIT_TEST
/* FUNCTION NAME : SYS_CALLBACK_MGR_SendMsg
 * PURPOSE:
 *      Send callback message according to callback_event_id.
 *
 * INPUT:
 *      src_csc_id        -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      callback_event_id -- callback event id
 *      sysfun_msg_p      -- callback message to be sent
 *      is_notify_ipcfail -- TRUE if need to notify the thread which receives the message when
 *                           fail to send a callback message, FALSE if do not need to notify.
 *
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      1. If the callback message is for passing packets to a CSC, it is tolerable to loss
 *         the packet if the destination queue is full. Need not to notify ipc fail to the target
 *         thread under this condition. However, the mref which contains the packet must be freed
 *         by the caller if the message is failed to send.
 *
 */
static BOOL_T SYS_CALLBACK_MGR_SendMsg(UI32_T src_csc_id, UI32_T callback_event_id, SYSFUN_Msg_T *sysfun_msg_p, BOOL_T is_notify_ipcfail)
{
    const UI32_T *msgqkey_list_p;
    SYS_CALLBACK_MGR_Sock_T *sockfd_list_p;
    SYSFUN_MsgQ_T msgq_handle;
    UI32_T sysfun_ret;
    BOOL_T ret=TRUE;

#if (SYS_CPNT_SYS_CALLBACK_SOCKET == TRUE)
/*  OpenFlow Agent needs the following two event
 *  SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_LINK_UP
 *  SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_LINK_DOWN
 */
    sockfd_list_p = SYS_CALLBACK_MGR_GetSocketList(callback_event_id);
    if (sockfd_list_p != NULL)
    {
        while (sockfd_list_p->socketfd != -1)
        {
            if((SYSFUN_OK != (sysfun_ret = SYSFUN_SendMsgToIPCSocket(sockfd_list_p->socketfd, sockfd_list_p->sock_filename,
                                                sysfun_msg_p))) && (TRUE == is_notify_ipcfail))
            {
                ret=FALSE;
            }
            sockfd_list_p++;
        }

    }
#endif

#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;

    if(SYS_CALLBACK_MGR_CheckRefineMsgqList(callback_event_id))
    {
       syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
       ret = SYS_CALLBACK_MGR_GenVlanList(callback_event_id,src_csc_id,is_notify_ipcfail,syscb_msg_p->callback_data);
       return ret;
    }
    if(SYS_CALLBACK_MGR_IslistMsgqList(callback_event_id))
    {
    /*get sub callback event id ,in order to get msgqkey list*/
      if(!(SYS_CALLBACK_MGR_GetSubCallbackEventId(sysfun_msg_p,&callback_event_id)))
      return FALSE;

    }
/* EPR:ES3628BT-FLF-ZZ-00096
    Problem:systmetest DUT display error msg wzhen power up
    Rootcause: 1, create vlan ,vlan add port will set bitmap to callback to other csc,and it will send later for the timer
                         2, add the port to trunk is behind port add to vlan,but the msg is sent before port add vlan for the time
    Solution:if the event is send by swctrl_group,if the queue has msg ,send the msg first
     Files:sys_callback_mgr.c ,igv3snp_l2.c
*/
    if(SYS_CALLBACK_IsSwitchGroupEventId(callback_event_id))
    {
        if(!SYS_CALLBACK_REFINED_OM_QueueIsEmpty())
          SYS_CALLBACK_MGR_ProcessRefineOmDB();
    }

#endif

    msgqkey_list_p=SYS_CALLBACK_MGR_GetMsgqKeyList(callback_event_id);
    if(msgqkey_list_p==NULL)
        return FALSE;

    sysfun_msg_p->msg_type = SYS_CALLBACK_MGR_DEFAULT_MSG_TYPE;
    sysfun_msg_p->cmd = SYS_MODULE_SYS_CALLBACK;

    while(*msgqkey_list_p!=0)
    {
        if(SYSFUN_OK==SYSFUN_GetMsgQ(*msgqkey_list_p, SYSFUN_MSGQ_BIDIRECTIONAL, &msgq_handle))
        {
            if((SYSFUN_OK!=(sysfun_ret=SYSFUN_SendRequestMsg(msgq_handle, sysfun_msg_p, SYSFUN_TIMEOUT_NOWAIT,
                SYSFUN_SYSTEM_EVENT_IPCMSG, 0, NULL))) && (TRUE==is_notify_ipcfail))
            {
                UI32_T task_id;

                SYS_CALLBACK_OM_UpdateFailEntry(*msgqkey_list_p, src_csc_id);
                task_id=SYSFUN_GetMsgQOwner(msgq_handle);

                if(task_id)
                    SYSFUN_SendEvent(task_id, SYSFUN_SYSTEM_EVENT_IPCFAIL);

                ret=FALSE;
            }
            SYS_CALLBACK_MGR_Backdoor_DebugIpc(src_csc_id, *msgqkey_list_p, sysfun_msg_p, sysfun_ret);
            SYSFUN_ReleaseMsgQ(msgq_handle);
        }
        msgqkey_list_p++;
    }

    return ret;
}

#else /* #ifndef UNIT_TEST */
static BOOL_T SYS_CALLBACK_MGR_SendMsg(UI32_T src_csc_id, UI32_T callback_event_id, SYSFUN_Msg_T *sysfun_msg_p, BOOL_T is_notify_ipcfail)
{
    const UI32_T *msgqkey_list_p;
    int i;

    sysfun_msg_p->msg_type = SYS_CALLBACK_MGR_DEFAULT_MSG_TYPE;
    sysfun_msg_p->cmd = SYS_MODULE_SYS_CALLBACK;

    printf("\nIn %s(): src_csc_id=%d, callback_event_id=%d\n", __FUNCTION__,
        (int)src_csc_id, (int)callback_event_id);
    printf("Dump message header:\n");
    printf("SYSFUN_Msg_T.msg_type=%d, cmd=%d, msg_size=%d\n",
        (int)sysfun_msg_p->msg_type, (int)sysfun_msg_p->cmd, (int)sysfun_msg_p->msg_size);
    printf("Dump message content:\n");
    for(i=0; i<sysfun_msg_p->msg_size; i++)
    {
        printf("%02X", *((sysfun_msg_p->msg_buf) + i));
        if(i%16==15)
            printf("\n");
        else
            printf(" ");
    }

    msgqkey_list_p=SYS_CALLBACK_MGR_GetMsgqKeyList(callback_event_id);
    if(msgqkey_list_p==NULL)
    {
        printf("SYS_CALLBACK_MGR_GetMsgqKeyList return FALSE. callback_event_id=%d\n",
            (int)callback_event_id);
        return FALSE;
    }

    printf("\nDump msgqkey list:\n");
    while(*msgqkey_list_p!=0)
    {
        printf("%d ", (int)*msgqkey_list_p);
        msgqkey_list_p++;
    }
    printf("\n");
    return TRUE;
}

#endif /* #ifndef UNIT_TEST */

/* FUNCTION NAME : SYS_CALLBACK_MGR_SendMsgWithMsgQKey
 * PURPOSE:
 *      Send callback message to the given msgq handle.
 *
 * INPUT:
 *      src_csc_id     -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      dest_msgq_key  -- key of the destination message queue
 *      sysfun_msg_p   -- callback message to be sent
 *      is_notify_ipcfail -- TRUE if need to notify the thread which receives the message when
 *                           fail to send a callback message, FALSE if do not need to notify.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      1. If the callback message is for passing packets to a CSC, it is tolerable to loss
 *         the packet if the destination queue is full. Need not to notify ipc fail to the target
 *         thread under this condition. However, the mref which contains the packet must be freed
 *         by the caller if the message is failed to send.
 *
 */
static BOOL_T SYS_CALLBACK_MGR_SendMsgWithMsgQKey(UI32_T src_csc_id, UI32_T dest_msgq_key, SYSFUN_Msg_T *sysfun_msg_p, BOOL_T is_notify_ipcfail)
{
    SYSFUN_MsgQ_T msgq_handle;
    UI32_T sysfun_ret;
    BOOL_T ret=TRUE;

    sysfun_msg_p->msg_type = SYS_CALLBACK_MGR_DEFAULT_MSG_TYPE;
    sysfun_msg_p->cmd = SYS_MODULE_SYS_CALLBACK;

    if(SYSFUN_OK==SYSFUN_GetMsgQ(dest_msgq_key, SYSFUN_MSGQ_BIDIRECTIONAL, &msgq_handle))
    {
        if((SYSFUN_OK!=(sysfun_ret=SYSFUN_SendRequestMsg(msgq_handle, sysfun_msg_p, SYSFUN_TIMEOUT_NOWAIT,
            SYSFUN_SYSTEM_EVENT_IPCMSG, 0, NULL))) && (TRUE==is_notify_ipcfail))
        {
            UI32_T task_id;

            SYS_CALLBACK_OM_UpdateFailEntry(dest_msgq_key, src_csc_id);
            task_id=SYSFUN_GetMsgQOwner(msgq_handle);

            if(task_id)
                SYSFUN_SendEvent(task_id, SYSFUN_SYSTEM_EVENT_IPCFAIL);

            ret=FALSE;
        }
        SYS_CALLBACK_MGR_Backdoor_DebugIpc(src_csc_id, dest_msgq_key, sysfun_msg_p, sysfun_ret);
        SYSFUN_ReleaseMsgQ(msgq_handle);
    }

    return ret;
}

/* FUNCTION NAME : SYS_CALLBACK_MGR_SendMsgWithFailCount
 * PURPOSE:
 *      Send callback message according to callback_event_id.
 *      Same as SYS_CALLBACK_MGR_SendMsg but with fail count output.
 *
 * INPUT:
 *      src_csc_id        -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      callback_event_id -- callback event id
 *      sysfun_msg_p      -- callback message to be sent
 *      is_notify_ipcfail -- TRUE if need to notify the thread which receives the message when
 *                           fail to send a callback message, FALSE if do not need to notify.
 *
 * OUTPUT:
 *      fail_count_p      -- output 0 if no error, otherwise output total failed IPC msgq number
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      1. If the callback message is for passing packets to a CSC, it is tolerable to loss
 *         the packet if the destination queue is full. Need not to notify ipc fail to the target
 *         thread under this condition. However, the mref which contains the packet must be freed
 *         by the caller if the message is failed to send.
 *
 */
static BOOL_T SYS_CALLBACK_MGR_SendMsgWithFailCount(UI32_T src_csc_id, UI32_T callback_event_id, SYSFUN_Msg_T *sysfun_msg_p,
                                                    BOOL_T is_notify_ipcfail, UI32_T *fail_count_p)
{
    const UI32_T *msgqkey_list_p;
    SYSFUN_MsgQ_T msgq_handle;
    UI32_T sysfun_ret;
    BOOL_T ret=TRUE;

#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;

    if(SYS_CALLBACK_MGR_CheckRefineMsgqList(callback_event_id))
    {
       syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
       ret = SYS_CALLBACK_MGR_GenVlanList(callback_event_id,src_csc_id,is_notify_ipcfail,syscb_msg_p->callback_data);
       return ret;
    }
    if(SYS_CALLBACK_MGR_IslistMsgqList(callback_event_id))
    {
    /*get sub callback event id ,in order to get msgqkey list*/
      if(!(SYS_CALLBACK_MGR_GetSubCallbackEventId(sysfun_msg_p,&callback_event_id)))
      return FALSE;

    }
/* EPR:ES3628BT-FLF-ZZ-00096
    Problem:systmetest DUT display error msg wzhen power up
    Rootcause: 1, create vlan ,vlan add port will set bitmap to callback to other csc,and it will send later for the timer
                         2, add the port to trunk is behind port add to vlan,but the msg is sent before port add vlan for the time
    Solution:if the event is send by swctrl_group,if the queue has msg ,send the msg first
     Files:sys_callback_mgr.c ,igv3snp_l2.c
*/
    if(SYS_CALLBACK_IsSwitchGroupEventId(callback_event_id))
    {
        if(!SYS_CALLBACK_REFINED_OM_QueueIsEmpty())
          SYS_CALLBACK_MGR_ProcessRefineOmDB();
    }

#endif

    msgqkey_list_p=SYS_CALLBACK_MGR_GetMsgqKeyList(callback_event_id);
    if(msgqkey_list_p==NULL)
        return FALSE;

    sysfun_msg_p->msg_type = SYS_CALLBACK_MGR_DEFAULT_MSG_TYPE;
    sysfun_msg_p->cmd = SYS_MODULE_SYS_CALLBACK;

    *fail_count_p = 0;
    while(*msgqkey_list_p!=0)
    {
        if(SYSFUN_OK==SYSFUN_GetMsgQ(*msgqkey_list_p, SYSFUN_MSGQ_BIDIRECTIONAL, &msgq_handle))
        {
            if((SYSFUN_OK!=(sysfun_ret=SYSFUN_SendRequestMsg(msgq_handle, sysfun_msg_p, SYSFUN_TIMEOUT_NOWAIT,
                SYSFUN_SYSTEM_EVENT_IPCMSG, 0, NULL))) && (TRUE==is_notify_ipcfail))
            {
                UI32_T task_id;

                SYS_CALLBACK_OM_UpdateFailEntry(*msgqkey_list_p, src_csc_id);
                task_id=SYSFUN_GetMsgQOwner(msgq_handle);

                if(task_id)
                    SYSFUN_SendEvent(task_id, SYSFUN_SYSTEM_EVENT_IPCFAIL);

                (*fail_count_p)++;
                ret = FALSE;
            }
            SYS_CALLBACK_MGR_Backdoor_DebugIpc(src_csc_id, *msgqkey_list_p, sysfun_msg_p, sysfun_ret);
            SYSFUN_ReleaseMsgQ(msgq_handle);
        }
        msgqkey_list_p++;
    }

    return ret;
}

#if (SYS_CPNT_SYS_CALLBACK_SOCKET == TRUE)

void SYS_CALLBACK_MGR_SetSocket(UI32_T module_id, int sockfd)
{
    FILE *fp;
    char opmode_conf;

    /* Only in OPENFLOW Mode can use these UNIX socket to send IPC msg */
    fp= fopen("/flash/of_opmode.conf", "r");
    if (fp)
    {
        opmode_conf = fgetc(fp);
        fclose(fp);

        if(opmode_conf != '2')
        {
            return;
        }
    }

    switch(module_id)
    {
        case SYS_MODULE_SWDRV:
            uport_link_updown_socket_list[0].socketfd = sockfd;
            break;
        case SYS_MODULE_L2MUX:
            l2mux_receive_of_packetin_socket_list[0].socketfd = sockfd;
            break;
        default:
            break;
    }
    return;
}
static SYS_CALLBACK_MGR_Sock_T* SYS_CALLBACK_MGR_GetSocketList(UI32_T callback_event_id)
{
    switch(callback_event_id)
    {
        /* SWDRV */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_LINK_UP:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_LINK_DOWN:
            return (SYS_CALLBACK_MGR_Sock_T*) &(uport_link_updown_socket_list[0]);

        /* L2MUX */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_OF_PACKET:
            return (SYS_CALLBACK_MGR_Sock_T*) &(l2mux_receive_of_packetin_socket_list[0]);
        default:
            break;
    }
    return (SYS_CALLBACK_MGR_Sock_T*) NULL;
}

#endif
/* FUNCTION NAME : SYS_CALLBACK_MGR_GetMsgqKeyList
 * PURPOSE:
 *      Get the message queue key list according to callback_event_id.
 *
 * INPUT:
 *      callback_event_id -- callback event id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NULL    --  Invalid callback_even_id
 *      Non-NULL--  The pointer to the msgqkey list.
 *
 * NOTES:
 *      None.
 *
 */
static const UI32_T* SYS_CALLBACK_MGR_GetMsgqKeyList(UI32_T callback_event_id)
{
    switch(callback_event_id)
    {
        /* AMTR */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_INTRUSION_MAC:
            return &(intrusionMac_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_MOVE:
            return &(port_move_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SECURITY_PORT_MOVE:
            return &(securityPortMove_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_AUTO_LEARN:
            return &(auto_learn_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_TABLE_DELETE_BY_PORT:
            return &(mac_table_delete_by_port_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_TABLE_DELETE_BY_VID:
            return &(mac_table_delete_by_vid_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_TABLE_DELETE_BY_VID_AND_PORT:
            return &(mac_table_delete_by_vid_and_port_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_TABLE_DELETE_BY_LIFE_TIME:
            return &(mac_table_delete_by_life_time_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_ADDR_UPDATE:
            return &(mac_addr_update_msgqkey_list[0]);

        /* VLAN */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_CREATE:
            return &(vlan_create_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_CREATE_FOR_GVRP:
            return &(vlan_create_for_gvrp_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_DESTROY:
            return &(vlan_destroy_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_DESTROY_FOR_GVRP:
            return &(vlan_destroy_for_gvrp_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_L3VLAN_DESTROY:
            return &(L3_vlan_destroy_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_ADD:
            return &(vlan_member_add_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_ADD_FOR_GVRP:
            return &(vlan_member_add_for_gvrp_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE:
            return &(vlan_member_delete_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE_FOR_GVRP:
            return &(vlan_member_delete_for_gvrp_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_PORT_MODE:
            return &(vlan_port_mode_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_ADD_FIRST_TRUNK_MEMBER:
            return &(finish_add_first_trunk_member_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_ADD_TRUNK_MEMBER:
            return &(finish_add_trunk_member_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_DELETE_TRUNK_MEMBER:
            return &(finish_delete_trunk_member_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_DELETE_LAST_TRUNK_MEMBER:
            return &(finish_delete_last_trunk_member_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE_BY_TRUNK:
            return &(vlan_member_delete_by_trunk_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PVID_CHANGE:
            return &(pvid_change_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_L3IF_OPER_STATUS_CHANGED:
            return &(l3if_oper_status_changed_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_ADD_FIRST_TRUNK_MEMBER:
            return &(add_first_trunk_member_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_ADD_TRUNK_MEMBER:
            return &(add_trunk_member_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_DELETE_TRUNK_MEMBER:
            return &(delete_trunk_member_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_DELETE_LAST_TRUNK_MEMBER:
            return &(delete_last_trunk_member_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_NAME_CHANGED:
            return &(vlan_name_changed_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PROTOVLAN_GID_BINDING_CHANGED:
            return &(protovlan_gid_binding_changed_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_TAG_CHANGED:
            return &(vlan_member_tag_changed_msgqkey_list[0]);

        /* XSTP */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_LPORT_ENTER_FORWARDING:
            return &(lport_enter_forwarding_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_LPORT_LEAVE_FORWARDING:
            return &(lport_leave_forwarding_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_LPORT_CHANGE_STATE:
            return &(lport_change_state_msgqkey_list[0]);
        /*add by Tony.Lei for IGMPSnooping */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_XSTP_LPORT_TC_CHANGE:
            return &(lport_tc_change_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_STP_CHANGE_VERSION:
            return &(stp_change_version_msgqkey_list[0]);

        /* AMTRDRV */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_NEW_MAC_ADDRESS:
            return &(announce_new_mac_address_msgqkey_list[0]);

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_AGING_OUT:
            return &(announce_agingout_mac_address_msgqkey_list[0]);

        /* NMTRDRV */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPDATE_LOCAL_NMTRDRV_STATS:
            return &(update_local_nmtrdrv_stats_msgqkey_list[0]);

        /* SWCTRL */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_OPER_UP:
            return &(port_oper_up_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_NOT_OPER_UP:
            return &(port_not_oper_up_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_ADMIN_ENABLE:
            return &(port_admin_enable_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_ADMIN_DISABLE:
            return &(port_admin_disable_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_SPEED_DUPLEX:
            return &(port_speed_duplex_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD_1ST:
            return &(trunk_member_add_1st_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD:
            return &(trunk_member_add_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE:
            return &(trunk_member_delete_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE_LST:
            return &(trunk_member_delete_lst_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_PORT_OPER_UP:
            return &(trunk_member_port_oper_up_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_PORT_NOT_OPER_UP:
            return &(trunk_member_port_not_oper_up_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ACTIVE:
            return &(trunk_member_active_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_INACTIVE:
            return &(trunk_member_inactive_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_SPEED_DUPLEX:
            return &(uport_speed_duplex_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_LINK_UP:
            return &(uport_link_up_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_LINK_DOWN:
            return &(uport_link_down_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_ADMIN_ENABLE:
            return &(uport_admin_enable_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_ADMIN_DISABLE:
            return &(uport_admin_disable_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_STATUS_CHANGED_PASSIVELY:
            return &(port_status_changed_passively_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_LINK_DOWN:
            return &(port_link_down_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_LINK_UP:
            return &(port_link_up_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_FAST_LINK_UP:
            return &(uport_fast_link_up_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_FAST_LINK_DOWN:
            return &(uport_fast_link_down_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_ADMIN_DISABLE_BEFORE:
            return &(port_admin_disable_before_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_LACP_EFFECTIVE_OPER_STATUS_CHANGED:
            return &(uport_lacp_effective_oper_status_changed_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_DOT1X_EFFECTIVE_OPER_STATUS_CHANGED:
            return &(uport_dot1x_effective_oper_status_changed_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_FORWARDING_UPORT_ADD_TO_TRUNK:
            return &(forwarding_uport_add_to_trunk_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_FORWARDING_TRUNK_MEMBER_DELETE:
            return &(forwarding_trunk_member_delete_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_FORWARDING_TRUNK_MEMBER_TO_NON_FORWARDING:
            return &(forwarding_trunk_member_to_non_forwarding_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_LPORT_TYPE_CHANGED:
            return &(lport_type_changed_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_TYPE_CHANGED:
            return &(uport_type_changed_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_IF_MAU_CHANGED:
            return &(if_mau_changed_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_LEARNING_STATUS_CHANGED:
            return &(port_learning_status_changed_msgqkey_list[0]);

        /* SWDRV */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_LINK_UP:
            return &(swdrv_port_link_up[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_LINK_DOWN:
            return &(swdrv_port_link_down[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_CRAFT_PORT_LINK_UP:
            return &(swdrv_craft_port_link_up[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_CRAFT_PORT_LINK_DOWN:
            return &(swdrv_craft_port_link_down[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_TYPE_CHANGED:
            return &(swdrv_port_type_changed[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_SPEED_DUPLEX:
            return &(swdrv_port_speed_duplex[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_FLOW_CTRL:
            return &(swdrv_port_flow_ctrl[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_HOT_SWAP_INSERT:
            return &(swdrv_hot_swap_insert[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_HOT_SWAP_REMOVE:
            return &(swdrv_hot_swap_remove[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_SFP_PRESENT:
            return &(swdrv_port_sfp_present[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_SFP_INFO:
            return &(swdrv_port_sfp_info[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_SFP_DDM_INFO:
            return &(swdrv_port_sfp_ddm_info[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_SFP_DDM_INFO_MEASURED:
            return &(swdrv_port_sfp_ddm_info_measured[0]);
#if (SYS_CPNT_CFM == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_CFM_DEFECT_NOTIFY:
            return &(cfm_defect_notify[0]);
#endif
        /* IGMPSNP */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_IGMPSNP_STATUS_CHANGED:
            return &(igmpsnp_status_changed_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ROUTER_PORT_ADD:
            return &(igmpsnp_router_port_add_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ROUTER_PORT_DELETE:
            return &(igmpsnp_router_port_delete_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_GROUP_MEMBER_ADD:
            return &(igmpsnp_group_member_add_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_GROUP_MEMBER_DELETE:
            return &(igmpsnp_group_member_delete_msgqkey_list[0]);

        /* SYSMGMT */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POWER_STATUS_CHANGED:
            return &(powerstatuschanged_msgqkey_list[0]);
#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_FAN_STATUS_CHANGED:
            return &(fanstatuschanged_msgqkey_list[0]);
#endif
#if (SYS_CPNT_THERMAL_DETECT == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_THERMAL_STATUS_CHANGED:
            return &(thermalstatuschanged_msgqkey_list[0]);
#endif
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_XFP_MODULE_STATUS_CHANGED:
            return &(xfpmodulestatuschanged_msgqkey_list[0]);

        /* SYSDRV */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_ALARM_INPUT_STATUS_CHANGED:
            return &(sysdrv_alarminputstatuschanged_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_MAJOR_ALARM_OUTPUT_STATUS_CHANGED:
            return &(sysdrv_alarmoutputstatuschanged_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_MINOR_ALARM_OUTPUT_STATUS_CHANGED:
            return &(sysdrv_alarmoutputstatuschanged_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_POWER_STATUS_CHANGED:
            return &(sysdrv_powerstatuschanged_msgqkey_list[0]);
#if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_POWER_TYPE_CHANGED:
            return &(sysdrv_powertypechanged_msgqkey_list[0]);
#endif
#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_FAN_STATUS_CHANGED:
            return &(sysdrv_fanstatuschanged_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_FAN_SPEED_CHANGED:
            return &(sysdrv_fanspeedchanged_msgqkey_list[0]);
#endif
#if (SYS_CPNT_THERMAL_DETECT == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_THERMAL_STATUS_CHANGED:
            return &(sysdrv_thermalstatuschanged_msgqkey_list[0]);
#endif
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_XFP_MODULE_STATUS_CHANGED:
            return &(sysdrv_xfpmodulestatuschanged_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_XENPAK_STATUS_CHANGED:
            return &(sysdrv_xenpakstatuschanged_msgqkey_list[0]);

        /* TRK */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ADD_STATIC_TRUNK_MEMBER:
            return &(add_static_trunk_member_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_DEL_STATIC_TRUNK_MEMBER:
            return &(del_static_trunk_member_msgqkey_list[0]);

        /* LAN */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_L2MUX_PACKET:
            return &(lan_receive_l2mux_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LACP_PACKET:
            return &(lan_receive_lacp_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_OAM_PACKET:
            return &(lan_receive_oam_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_OAM_LBK_PACKET:
            return &(lan_receive_oam_lbk_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LOOPBACK_PACKET:
            return &(lan_receive_loopback_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_DOT1X_PACKET:
            return &(lan_receive_dot1x_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_SFLOW_PACKET:
            return &(lan_receive_sflow_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LBD_PACKET:
            return &(lan_receive_lbd_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_UDLD_PACKET:
            return &(lan_receive_udld_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_ESMC_PACKET:
            return &(lan_receive_esmc_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_MLAG_PACKET:
            return &(lan_receive_mlag_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_AUTHENTICATION_DISPATCH:
            return &(lan_authentication_dispatch_msgqkey_list[0]);
        /* ISC */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_STKTPLG_PACKET:
            return &(isc_receive_stktplg_packet_msgqkey_list[0]);

        /* L2_MUX */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_STA_PACKET:
            return &(l2mux_receive_sta_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_GVRP_PACKET:
            return &(l2mux_receive_gvrp_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LLDP_PACKET:
            return &(l2mux_receive_lldp_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_IGMPSNP_PACKET:
            return &(l2mux_receive_igmpsnp_packet_msgqkey_list[0]);

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_MLDSNP_PACKET:
            return &(l2mux_receive_mldsnp_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_CFM_PACKET:
            return &(l2mux_receive_cfm_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_ELPS_PACKET:
            return &(l2mux_receive_elps_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_PPPOED_PACKET:
            return &(l2mux_receive_pppoed_packet_msgqkey_list[0]);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_RAPS_PACKET:
            return &(l2mux_receive_raps_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_ERPS_HEALTH_PACKET:
            return &(l2mux_receive_erps_health_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_CLUSTER_PACKET:
            return &(l2mux_receive_cluster_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_PTP_PACKET:
            return &(l2mux_receive_ptp_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RX_SNOOP_DHCP_PACKET:
            return &(l2mux_rx_snoop_dhcp_packet_msgqkey_list[0]);
        /* IML */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_IP_PACKET:
            return &(l2mux_receive_ip_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_BOOTP_PACKET:
            return &(iml_receive_bootp_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_UDP_HELPER_PACKET:
            return &(iml_receive_udp_helper_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_ARP_PACKET:
            return &(iml_receive_arp_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_HSRP_PACKET:
            return &(iml_receive_hsrp_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_VRRP_PACKET:
            return &(iml_receive_vrrp_packet_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TX_SNOOP_DHCP_PACKET:
            return &(iml_tx_snoop_dhcp_packet_msgqkey_list[0]);
#if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_RAGUARD_PACKET:
            return &(iml_receive_raguard_packet_msgqkey_list[0]);
#endif
#if(SYS_CPNT_DHCPV6SNP == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_DHCPV6SNP_PACKET:
            return &(iml_receive_dhcpv6snp_packet_msgqkey_list[0]);
#endif
        /* STKTPLG */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_STACK_STATE:
            return &(stack_state_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MODULE_STATE_CHANGED:
            return &(module_state_changed_msgqkey_list[0]);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UnitHotInsertRemove:
            return &(unit_hot_swap_msgqkey_list[0]);
#endif
        /* STKCTRL */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SAVING_CONFIG_STATUS:
            return &(saving_config_status_msgqkey_list[0]);

        /* NETCFG */
        /* Triggered by IPCFG
         */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_CREATED:
            return &(rif_created_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_ACTIVE:
            return &(rif_active_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_DOWN:
            return &(rif_down_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_DESTROYED:
            return &(rif_destroyed_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_IPCFG_NSM_ROUTE_CHANGE:
            return &(nsm_route_change_msgqkey_list[0]);

        /*Donny.li modify for VRRP */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_L3IF_DESTROY:
            return &(netcfg_l3if_destroy_msgqkey_list[0]);
        /*Donny.li modify for VRRP end*/
#if (SYS_CPNT_DHCPV6 == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_IPV6_ADDRAUTOCONFIG:
            return &(netcfg_ipv6_addrautoconfig_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_L3IF_CREATE:
            return &(netcfg_l3if_create_msgqkey_list[0]);

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_L3IF_OPER_STATUS_UP:
            return &(netcfg_l3if_oper_status_up_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_L3IF_OPER_STATUS_DOWN:
            return &(netcfg_l3if_oper_status_down_msgqkey_list[0]);
#endif
        /* CLI */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PROVISION_COMPLETE:
            return &(provision_complete_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MODULE_PROVISION_COMPLETE:
            return &(module_provision_complete_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ENTER_TRANSITION_MODE:
            return &(enter_transition_mode_msgqkey_list[0]);

        /* LLDP */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TELEPHONE_DETECT:
            return &(telephone_detect_msgqkey_list[0]);
#ifdef SYS_CPNT_POE_PSE_DOT3AT
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_DOT3AT_INFO_RECEIVED:
            return &(dot3at_info_received_msgqkey_list[0]);
#endif
#if (SYS_CPNT_CN == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_CN_REMOTE_CHANGE:
            return &(cn_remote_change_msgqkey_list[0]);
#endif

        /* AMTRL3 */
#if (SYS_CPNT_AMTRL3 == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_NEXTHOP_STATUS_CHANGE:
            return &(nexthop_status_change_msgqkey_list[0]);
#endif

/*maggie liu for RADIUS authentication ansync*/
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RADIUS_AUTH_RESULT:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RADIUS_AUTH_RESULT_2:
            return &(radius_authen_result_msgqkey_list[0]);

#if (SYS_CPNT_DHCP == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_DHCP_RESTART3:
            return &(dhcp_restart3_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_DHCP_SET_IF_ROLE:
            return &(dhcp_setifrole_msgqkey_list[0]);
              case SYS_CALLBACK_MGR_CALLBACKEVENTID_DHCP_SET_IF_STATUS:
                    return &(dhcp_setifstatus_msgqkey_list[0]);
#endif

#if (SYS_CPNT_CLI_DYNAMIC_PROVISION_VIA_DHCP== TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_CLI_DYNAMIC_PROVISION_VIA_DHCP:
            return &(dhcp_rxoptionconfig_msgqkey_list[0]);
#endif

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RELOAD_REMAINING_TIME:
            return &(sysmgmt_announce_remain_date_msgqkey_list[0]);
#endif
#if (SYS_CPNT_IP_TUNNEL == TRUE)
    case  SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_IPV6_PACKET:
        return &(netcfg_announce_ipv6_msgqkey_list[0]);
    case  SYS_CALLBACK_MGR_CALLBACKEVENTID_TUNNEL_NET_ROUTE_HIT_BIT_CHANGE:
        return &(amtrl3_tunnel_net_route_hit_bit_change_msgqkey_list[0]);
#endif /*SYS_CPNT_IP_TUNNEL*/

#if (SYS_CPNT_POE == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_DETECTION_STATUS_CHANGE:
            return &(poedrv_port_detection_status_change_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_STATUS_CHANGE:
            return &(poedrv_port_status_change_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_IS_MAIN_POWER_REACH_MAXIMUM:
            return &(poedrv_is_main_power_reach_maximum_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_OVERLOAD_STATUS_CHANGE:
            return &(poedrv_port_overload_status_change_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_POWER_CONSUMPTION_CHANGE:
            return &(poedrv_port_power_consumption_change_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_POWER_CLASSIFICATION_CHANGE:
            return &(poedrv_port_power_classification_change_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_MAIN_PSE_CONSUMPTION_CHANGE:
            return &(poedrv_main_pse_consumption_change_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PSE_OPER_STATUS_CHANGE:
            return &(poedrv_pse_oper_status_change_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_POWER_DENIED_OCCUR_FRENQUENTLY:
            return &(poedrv_power_denied_occur_frenquently_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_FAILURE_STATUS_CHANGE:
            return &(poedrv_port_failure_status_change_msgqkey_list[0]);
#endif

#if (SYS_CPNT_CLUSTER == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_CLUSTER_CHANGEROLE:
            return &(cluster_changerole_msgqkey_list[0]);
#endif

#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ACL_DELETED:
            return &(acl_deleted_msgqkey_list[0]);

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POLICY_MAP_DELETED:
            return &(policy_map_deleted_msgqkey_list[0]);
#endif /* #if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE) */

#if (SYS_CPNT_COS == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_COS_PORT_CONFIG_CHANGED:
            return &(cos_port_config_changed_msgqkey_list[0]);
#endif /* #if (SYS_CPNT_COS == TRUE) */

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MGMT_IP_FLT_CHANGED:
            return &(mgmt_ip_flt_changed_msgqkey_list[0]);
#endif /* #if (SYS_CPNT_MGMT_IP_FLT == TRUE) */

        /* CMGR */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SET_PORT_STATUS:
            return &(set_port_status_msgqkey_list[0]);

#if (SYS_CPNT_DCBX == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_ETS_TLV:
            return &(ets_received_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_PFC_TLV:
            return &(pfc_received_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ETS_CFG_CHANGED:
            return &(ets_cfg_changed_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PFC_CFG_CHANGED:
            return &(pfc_cfg_changed_msgqkey_list[0]);
#endif  /* #if (SYS_CPNT_DCBX == TRUE) */

#if (SYS_CPNT_IGMPAUTH == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RADIUS_IGMPAUTH_RESULT:
            return &(igmp_msgqkey_list[0]);
#endif
        /* SYS_CALLBACK_GROUP */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RX_DHCP_PACKET:
            return &(sys_callback_rx_dhcp_packet_msgqkey_list[0]);
#if (SYS_CPNT_PBR == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_HOST_ROUTE_CHANGED:
            return &(host_route_changed_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ACL_CHANGED:
            return &(acl_changed_msgqkey_list[0]);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ROUTEMAP_CHANGED:
            return &(routemap_changed_msgqkey_list[0]);
#endif

        default:
            break;
    }
    return NULL;
}

#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)


static void SYS_CALLBACK_MGR_GetPosition(UI32_T index, UI32_T *list_index, UI32_T *list_position)
{
    *list_index =   (UI32_T)(index - 1) / 8;
    *list_position = (UI32_T)(index - 1) % 8;

    return;

} /* end of VLAN_OM_GetPosition() */
/* FUNCTION NAME : SYS_CALLBACK_MGR_GenVlanList
 * PURPOSE:
 *    refine the message,set it to bitmap,and store the new message in the database,in order to send the new
      message later
 *
 * INPUT: UI32_T callback_event_id,
                  UI32_T src_csc_id,
                  BOOL_T is_notify_ipcfail,
                 UI8_T*  callback_data
 *    *
 * OUTPUT: *      None.
 *
 * RETURN: *      TRUE    --  SUCCESS
 *                            FALSE--  FAILED
 *
 * NOTES: *      None.
 *
 */

static const BOOL_T SYS_CALLBACK_MGR_GenVlanList(UI32_T callback_event_id,
                                                                         UI32_T src_csc_id,
                                                                         BOOL_T is_notify_ipcfail,
                                                                         UI8_T*  callback_data)
{
  SYS_CALLBACK_MGR_VlanMemberAdd_CBData_T  *vlanmemaddcbdata;
  SYS_CALLBACK_MGR_VlanCreate_CBData_T  *vlancreatecbdata;
  SYS_CALLBACK_ARG_ENTRY_INFO entry,tementry;
  UI32_T     byte, shift,vid;
  BOOL_T ret =TRUE,flag = FALSE;
  UI16_T position;

  if(!SYS_CALLBACK_MGR_IsVlanlistEventId(callback_event_id))
    return TRUE;


  if(callback_data == NULL)
   return FALSE;

   memset(&entry,0,sizeof(SYS_CALLBACK_ARG_ENTRY_INFO));

  switch(callback_event_id){

   case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_CREATE:
      vlancreatecbdata= (SYS_CALLBACK_MGR_VlanCreate_CBData_T*)callback_data;
      entry.allarg.arg.arg1.value = vlancreatecbdata->vlan_status;
      VLAN_IFINDEX_CONVERTTO_VID(vlancreatecbdata->vid_ifindex, vid);

      break;
   case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE:
   case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_ADD:
     vlanmemaddcbdata= (SYS_CALLBACK_MGR_VlanMemberAdd_CBData_T*)callback_data;
     entry.allarg.arg.arg2.value[0] = vlanmemaddcbdata->lport_ifindex;
     entry.allarg.arg.arg2.value[1] = vlanmemaddcbdata->vlan_status;
     VLAN_IFINDEX_CONVERTTO_VID(vlanmemaddcbdata->vid_ifindex, vid);
     break;
   default:
     return TRUE;
  }

  SYS_CALLBACK_MGR_GetPosition(vid, &byte, &shift);
  entry.src_csc_id = src_csc_id;
  entry.callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_LIST;
  entry.sub_callback_event_id = callback_event_id;
  entry.is_notify_ipcfail = is_notify_ipcfail;
  entry.allarg.list.vlanlist[byte] |=  ((0x01) << (7 - shift));



  /*queue is empty,add the entry*/
  if(SYS_CALLBACK_REFINED_OM_QueueIsEmpty())
  {

    ret = SYS_CALLBACK_REFINED_OM_EnQueue(&entry);
  }
  else/*lookup if there is the entry the same as it and insert it*/
  {
     position =SYS_CALLBACK_REFINED_OM_GetHead();
     while(SYS_CALLBACK_REFINED_OM_GetQueuePointedData(position,&tementry))
     {
        if(tementry.callback_event_id ==SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_LIST &&
          tementry.sub_callback_event_id == callback_event_id)
        {
           switch(callback_event_id)
           {  case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_CREATE:
               if(tementry.allarg.arg.arg1.value ==vlancreatecbdata->vlan_status)
                  flag = TRUE;
                break;
             case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE:
             case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_ADD:
              if(tementry.allarg.arg.arg2.value[0] ==vlanmemaddcbdata->lport_ifindex &&
             tementry.allarg.arg.arg2.value[1] ==vlanmemaddcbdata->vlan_status )
               flag = TRUE;
               break;
             default:
               return TRUE;
           }
           if(flag)
           {
             tementry.allarg.list.vlanlist[byte] |=  ((0x01) << (7 - shift));
             ret=SYS_CALLBACK_REFINED_OM_SetQueuePointedData(position,tementry);
             return ret;
           }
        }
        if(!flag)
        {
         /*the entry is not the same as the before,so send the entry in the queue*/
         SYS_CALLBACK_MGR_ProcessRefineOmDB();
        }
        position=(position+1)%SYS_CALLBACK_MAX_ENTRY;
     }

    ret = SYS_CALLBACK_REFINED_OM_EnQueue(&entry);
  }
  return ret;
}
/* FUNCTION NAME : SYS_CALLBACK_MGR_GetRefineMsgqList
 * PURPOSE:
 *     check if the type of callback_event_id need to changed  bitmap to set
 *
 * INPUT:
 *      callback_event_id -- callback event id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    --  need to refined to bitmap
 *      FALSE--  not need to refined to bitmap
 *
 * NOTES:
 *      None.
 *
 */

static const BOOL_T SYS_CALLBACK_MGR_CheckRefineMsgqList(UI32_T callback_event_id)
{
  BOOL_T ret;

   ret =SYS_CALLBACK_MGR_IsVlanlistEventId(callback_event_id);
   if(ret)
    return ret;
   else
     ret =SYS_CALLBACK_MGR_IsPortlistEventId(callback_event_id);

  return ret;
}
/* FUNCTION NAME : SYS_CALLBACK_MGR_ProcessRefineOmDB
 * PURPOSE:
 *    when the timer arrive, send the message stored in the database
 *
 * INPUT: NONE
 *    *
 * OUTPUT: *      None.
 *
 * RETURN: *      TRUE    --  SUCCESS
 *                            FALSE--  FAILED
 *
 * NOTES: *      None.
 *
 */

BOOL_T SYS_CALLBACK_MGR_ProcessRefineOmDB()
{
    SYSFUN_Msg_T    *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    SYS_CALLBACK_MGR_REFINEList_CBData_T  *cbdata_msg_p;
    UI8_T   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_REFINEList_CBData_T)))];
    SYS_CALLBACK_ARG_ENTRY_INFO entry;
    BOOL_T ret = FALSE;
    int i;

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_REFINEList_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_REFINEList_CBData_T*)syscb_msg_p->callback_data;


  while(!SYS_CALLBACK_REFINED_OM_QueueIsEmpty())
  {
    SYS_CALLBACK_REFINED_OM_DeQueue(&entry);
    syscb_msg_p->callback_event_id = entry.callback_event_id;
    syscb_msg_p->sub_callback_event_id = entry.sub_callback_event_id;

    memcpy(cbdata_msg_p,&(entry.allarg),sizeof(SYS_CALLBACK_MGR_REFINEList_CBData_T));
    ret=SYS_CALLBACK_MGR_SendMsg(entry.src_csc_id, SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_LIST, sysfun_msg_p, entry.is_notify_ipcfail);

  }
   return ret;
}
static BOOL_T SYS_CALLBACK_MGR_GetSubCallbackEventId(SYSFUN_Msg_T *sysfun_msg_p,UI32_T *sub_callback_event_id)
{
  SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
  SYS_CALLBACK_MGR_REFINEList_CBData_T  *cbdata_msg_p;

  if(sysfun_msg_p == FALSE)
    return FALSE;

  syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
  *sub_callback_event_id = syscb_msg_p->sub_callback_event_id;
  return TRUE;

}
static const BOOL_T SYS_CALLBACK_MGR_IslistMsgqList(UI32_T callback_event_id)
{
  if(callback_event_id ==SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_LIST )
   return TRUE;
  else
   return FALSE;
}
static const BOOL_T SYS_CALLBACK_MGR_IsVlanlistEventId(UI32_T callback_event_id)
{
    switch(callback_event_id)
  {
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_ADD:
      return TRUE;
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE:
      return TRUE;
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_CREATE:
      return TRUE;
    default :
      break;
  }
  return FALSE;
}
static const BOOL_T SYS_CALLBACK_MGR_IsPortlistEventId(UI32_T callback_event_id)
{
    switch(callback_event_id)
  {
    default :
      break;
  }
  return FALSE;
}

static const BOOL_T SYS_CALLBACK_IsSwitchGroupEventId(UI32_T callback_event_id)
{

  switch(callback_event_id)
  {
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_DESTROY:
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_L3VLAN_DESTROY:
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_PORT_MODE:
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_ADD_FIRST_TRUNK_MEMBER:
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_ADD_TRUNK_MEMBER:
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_DELETE_TRUNK_MEMBER:
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_DELETE_LAST_TRUNK_MEMBER:
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE_BY_TRUNK:
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_PVID_CHANGE:
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_L3IF_OPER_STATUS_CHANGED:
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_ADD_FIRST_TRUNK_MEMBER:
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_ADD_TRUNK_MEMBER:
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_DELETE_TRUNK_MEMBER:
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_DELETE_LAST_TRUNK_MEMBER:
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_NAME_CHANGED:
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_PROTOVLAN_GID_BINDING_CHANGED:
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_TAG_CHANGED:
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_ADD:
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE:
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_CREATE:
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_OPER_UP:                              /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_NOT_OPER_UP:                          /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_ADMIN_ENABLE:                         /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_ADMIN_DISABLE:                        /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_SPEED_DUPLEX:                         /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD_1ST:                      /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD:                          /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE:                       /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE_LST:                   /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_PORT_OPER_UP:                 /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_PORT_NOT_OPER_UP:             /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ACTIVE:                       /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_INACTIVE:                     /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_SPEED_DUPLEX:                        /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_LINK_UP:                             /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_LINK_DOWN:                           /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_ADMIN_ENABLE:                        /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_ADMIN_DISABLE:                       /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_STATUS_CHANGED_PASSIVELY:             /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_LINK_DOWN:                            /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_LINK_UP:                              /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_FAST_LINK_UP:                        /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_FAST_LINK_DOWN:                      /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_ADMIN_DISABLE_BEFORE:                 /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_LACP_EFFECTIVE_OPER_STATUS_CHANGED:  /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_DOT1X_EFFECTIVE_OPER_STATUS_CHANGED: /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_FORWARDING_UPORT_ADD_TO_TRUNK:             /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_FORWARDING_TRUNK_MEMBER_DELETE:            /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_FORWARDING_TRUNK_MEMBER_TO_NON_FORWARDING: /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_LPORT_TYPE_CHANGED:                        /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_TYPE_CHANGED:                        /* SWCTRL */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_IF_MAU_CHANGED:
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_LEARNING_STATUS_CHANGED:
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_ADD_STATIC_TRUNK_MEMBER:                   /* TRK */
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_DEL_STATIC_TRUNK_MEMBER:                   /* TRK */
      return TRUE;
    default:
      return FALSE;

  }
  return FALSE;
}

#endif

#if (SYS_CPNT_CLUSTER==TRUE)


/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveClusterPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When LAN received packet for cluster, it will notify cluster with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      tag_info       --- packet tag info
 *      type           --- packet type
 *      pkt_length     --- packet length
 *      lport          --- source lport id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveClusterPacketCallback(UI32_T         src_csc_id,
                                                     L_MM_Mref_Handle_T *mref_handle_p,
                                                     UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                     UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                     UI16_T         tag_info,
                                                     UI16_T         type,
                                                     UI32_T         pkt_length,
                                                     UI32_T         lport)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                      fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_CLUSTER_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_CLUSTER_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->tag_info           = tag_info;
    cbdata_msg_p->type               = type;
    cbdata_msg_p->pkt_length         = pkt_length;
    cbdata_msg_p->lport              = lport;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if( FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ChangeClusterRoleCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When core layer change cluster role, it will notify snmp with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      role           --- cluster role
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ChangeClusterRoleCallback(UI32_T   src_csc_id,
                                                  UI32_T   role )
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_CLUSTER_ChangeRole_CBData_T)))];
    SYSFUN_Msg_T                                *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                      *syscb_msg_p;
    SYS_CALLBACK_MGR_CLUSTER_ChangeRole_CBData_T  *cbdata_msg_p;


    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_CLUSTER_ChangeRole_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_CLUSTER_ChangeRole_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_CLUSTER_CHANGEROLE;
    cbdata_msg_p->role = role;


    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

}

#endif

#if (SYS_CPNT_DHCP == TRUE)
BOOL_T SYS_CALLBACK_MGR_DHCPRestart3Callback(UI32_T src_csc_id,
                                                                  UI32_T restart_object)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_DHCP_Restart3_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                       *syscb_msg_p;
    SYS_CALLBACK_MGR_DHCP_Restart3_CBData_T      *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_DHCP_Restart3_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_DHCP_Restart3_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_DHCP_RESTART3;
    cbdata_msg_p->restart_object = restart_object;
    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

BOOL_T SYS_CALLBACK_MGR_DHCPSetIfRoleCallback(UI32_T src_csc_id,
                                                                  UI32_T vid_ifindex,
                                                                  UI32_T role)
{

    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_DHCP_SetIfRole_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                       *syscb_msg_p;
    SYS_CALLBACK_MGR_DHCP_SetIfRole_CBData_T     *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_DHCP_SetIfRole_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_DHCP_SetIfRole_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_DHCP_SET_IF_ROLE;
    cbdata_msg_p->vid_ifindex = vid_ifindex;
    cbdata_msg_p->role = role;
    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}
BOOL_T SYS_CALLBACK_MGR_DHCPSetIfStatusCallback(UI32_T src_csc_id,
                                                                  UI32_T vid_ifindex,
                                                                  UI32_T status)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_DHCP_SetIfStatud_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                       *syscb_msg_p;
    SYS_CALLBACK_MGR_DHCP_SetIfStatud_CBData_T     *cbdata_msg_p;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_DHCP_SetIfStatud_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_DHCP_SetIfStatud_CBData_T*)syscb_msg_p->callback_data;
    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_DHCP_SET_IF_STATUS;
    cbdata_msg_p->vid_ifindex = vid_ifindex;
    cbdata_msg_p->status = status;
    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}
#endif

#if (SYS_CPNT_CLI_DYNAMIC_PROVISION_VIA_DHCP== TRUE)
BOOL_T SYS_CALLBACK_MGR_DHCP_RxOptionConfigCallback(UI32_T src_csc_id,
                                                                  UI32_T option66_length,
                                                                  UI8_T  *option66_data,
                                                                  UI32_T option67_length,
                                                                  char   *option67_data)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_DHCP_RxOptionConfig_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                       *syscb_msg_p;
    SYS_CALLBACK_MGR_DHCP_RxOptionConfig_CBData_T     *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_DHCP_RxOptionConfig_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_DHCP_RxOptionConfig_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_CLI_DYNAMIC_PROVISION_VIA_DHCP;
    cbdata_msg_p->option66_length = option66_length;
    cbdata_msg_p->option67_length = option67_length;
    memcpy(cbdata_msg_p->option66_data, option66_data, SYS_ADPT_DHCP_MAX_DOMAIN_NAME_LEN);
    memcpy(cbdata_msg_p->option67_data, option67_data, SYS_ADPT_DHCP_MAX_BOOTFILE_NAME_LEN);

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}
#endif

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
BOOL_T SYS_CALLBACK_MGR_AnnounceReloadReaminTime(
    UI32_T src_csc_id,
    UI32_T remain_minutes)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AnnouceReloadRemainTime_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                       *syscb_msg_p;
    SYS_CALLBACK_MGR_AnnouceReloadRemainTime_CBData_T *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AnnouceReloadRemainTime_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_AnnouceReloadRemainTime_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RELOAD_REMAINING_TIME;
    cbdata_msg_p->remaining_minutes = remain_minutes;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}
#endif

#if (SYS_CPNT_IP_TUNNEL == TRUE)
BOOL_T SYS_CALLBACK_MGR_AnnounceIpv6PacketCallback(UI32_T  src_csc_id,UI8_T* src_addr,UI8_T* dst_addr)
{
    UI8_T         ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AnnouceIpv6Packet_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                         *syscb_msg_p;
    SYS_CALLBACK_MGR_AnnouceIpv6Packet_CBData_T *cbdata_msg_p;


    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AnnouceIpv6Packet_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_AnnouceIpv6Packet_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_IPV6_PACKET;
    memcpy(cbdata_msg_p->source_address,src_addr,SYS_ADPT_IPV6_ADDR_LEN);
    memcpy(cbdata_msg_p->destination_address,dst_addr,SYS_ADPT_IPV6_ADDR_LEN);

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

BOOL_T SYS_CALLBACK_MGR_TunnelNetRouteHitBitChangeCallback(UI32_T  src_csc_id,UI32_T fib_id, UI8_T *dst_addr, UI32_T preflen, UI32_T unit_id)
{
    UI8_T         ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_TunnelNetRouteHitBitChange_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                         *syscb_msg_p;
    SYS_CALLBACK_MGR_TunnelNetRouteHitBitChange_CBData_T *cbdata_msg_p;


    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_TunnelNetRouteHitBitChange_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_TunnelNetRouteHitBitChange_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_TUNNEL_NET_ROUTE_HIT_BIT_CHANGE;
    cbdata_msg_p->fib_id   = fib_id;
    cbdata_msg_p->unit_id  = unit_id;
    cbdata_msg_p->preflen  = preflen;
    memcpy(cbdata_msg_p->dst_addr,dst_addr,SYS_ADPT_IPV6_ADDR_LEN);

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}
#endif /*SYS_CPNT_IP_TUNNEL*/

#if (SYS_CPNT_POE == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_POEDRV_PortDetectionStatusChange
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when detection status change
 * INPUT   : src_csc_id -- The csc_id who triggers this event (SYS_MODULE_POEDRV)
 *           unit       -- specified unit
 *           port       -- specified port
 *           status     -- port detection status
 * OUTPUT  : None.
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 * NOTES   : This api only used by poedrv send msg to poe module
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_POEDRV_PortDetectionStatusChange(UI32_T src_csc_id, UI32_T unit, UI32_T port, UI32_T status)
{
    SYSFUN_Msg_T                                        *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                    *syscb_msg_p;
    SYS_CALLBACK_MGR_POEDRV_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_POEDRV_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_POEDRV_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_POEDRV_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_DETECTION_STATUS_CHANGE;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;
    cbdata_msg_p->value = status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_POEDRV_PortStatusChange
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when detection status change
 * INPUT   : src_csc_id -- The csc_id who triggers this event (SYS_MODULE_POEDRV)
 *           unit       -- specified unit
 *           port       -- specified port
 *           status     -- actual status
 * OUTPUT  : None.
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 * NOTES   : This api only used by poedrv send msg to poe module
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_POEDRV_PortStatusChange(UI32_T src_csc_id, UI32_T unit, UI32_T port, UI32_T status)
{
    SYSFUN_Msg_T                                        *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                    *syscb_msg_p;
    SYS_CALLBACK_MGR_POEDRV_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_POEDRV_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_POEDRV_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_POEDRV_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_STATUS_CHANGE;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;
    cbdata_msg_p->value = status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_POEDRV_IsMainPowerReachMaximun
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when detection status change
 * INPUT   : src_csc_id -- The csc_id who triggers this event (SYS_MODULE_POEDRV)
 *           unit       -- specified unit
 *           status     -- actual status
 * OUTPUT  : None.
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 * NOTES   : This api only used by poedrv send msg to poe module
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_POEDRV_IsMainPowerReachMaximun(UI32_T src_csc_id, UI32_T unit, UI32_T status)
{
    SYSFUN_Msg_T                                        *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                    *syscb_msg_p;
    SYS_CALLBACK_MGR_POEDRV_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_POEDRV_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_POEDRV_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_POEDRV_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_IS_MAIN_POWER_REACH_MAXIMUM;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->value = status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_POEDRV_PortOverloadStatusChange
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when detection status change
 * INPUT   : src_csc_id -- The csc_id who triggers this event (SYS_MODULE_POEDRV)
 *           unit       -- specified unit
 *           port       -- specified port
 *           status     -- is port overload or not
 * OUTPUT  : None.
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 * NOTES   : This api only used by poedrv send msg to poe module
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_POEDRV_PortOverloadStatusChange(UI32_T src_csc_id, UI32_T unit, UI32_T port, UI32_T status)
{
    SYSFUN_Msg_T                                        *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                    *syscb_msg_p;
    SYS_CALLBACK_MGR_POEDRV_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_POEDRV_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_POEDRV_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_POEDRV_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_OVERLOAD_STATUS_CHANGE;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;
    cbdata_msg_p->value = status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_POEDRV_PortPowerConsumptionChange
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when detection status change
 * INPUT   : src_csc_id -- The csc_id who triggers this event (SYS_MODULE_POEDRV)
 *           unit       -- specified unit
 *           port       -- specified port
 *           power      -- port power consumption
 * OUTPUT  : None.
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 * NOTES   : This api only used by poedrv send msg to poe module
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_POEDRV_PortPowerConsumptionChange(UI32_T src_csc_id, UI32_T unit, UI32_T port, UI32_T power)
{
    SYSFUN_Msg_T                                        *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                    *syscb_msg_p;
    SYS_CALLBACK_MGR_POEDRV_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_POEDRV_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_POEDRV_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_POEDRV_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_POWER_CONSUMPTION_CHANGE;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;
    cbdata_msg_p->value = power;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_POEDRV_PortPowerClassificationChange
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when detection status change
 * INPUT   : src_csc_id -- The csc_id who triggers this event (SYS_MODULE_POEDRV)
 *           unit       -- specified unit
 *           port       -- specified port
 *           class      -- port power classification
 * OUTPUT  : None.
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 * NOTES   : This api only used by poedrv send msg to poe module
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_POEDRV_PortPowerClassificationChange(UI32_T src_csc_id, UI32_T unit, UI32_T port, UI32_T classification)
{
    SYSFUN_Msg_T                                        *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                    *syscb_msg_p;
    SYS_CALLBACK_MGR_POEDRV_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_POEDRV_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_POEDRV_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_POEDRV_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_POWER_CLASSIFICATION_CHANGE;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;
    cbdata_msg_p->value = classification;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_POEDRV_MainPseConsumptionChange
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when detection status change
 * INPUT   : src_csc_id -- The csc_id who triggers this event (SYS_MODULE_POEDRV)
 *           unit       -- specified unit
 *           power      -- pse power consumption
 * OUTPUT  : None.
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 * NOTES   : This api only used by poedrv send msg to poe module
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_POEDRV_MainPseConsumptionChange(UI32_T src_csc_id, UI32_T unit, UI32_T power)
{
    SYSFUN_Msg_T                                        *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                    *syscb_msg_p;
    SYS_CALLBACK_MGR_POEDRV_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_POEDRV_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_POEDRV_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_POEDRV_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_MAIN_PSE_CONSUMPTION_CHANGE;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->value = power;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_POEDRV_PseOperStatusChange
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when detection status change
 * INPUT   : src_csc_id -- The csc_id who triggers this event (SYS_MODULE_POEDRV)
 *           unit       -- specified unit
 *           status     -- pse operaation status
 * OUTPUT  : None.
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 * NOTES   : This api only used by poedrv send msg to poe module
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_POEDRV_PseOperStatusChange(UI32_T src_csc_id, UI32_T unit, UI32_T status)
{
    SYSFUN_Msg_T                                        *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                    *syscb_msg_p;
    SYS_CALLBACK_MGR_POEDRV_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_POEDRV_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_POEDRV_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_POEDRV_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PSE_OPER_STATUS_CHANGE;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->value = status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_POEDRV_PowerDeniedOccurFrequently
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when detection status change
 * INPUT   : src_csc_id -- The csc_id who triggers this event (SYS_MODULE_POEDRV)
 *           unit       -- specified unit
 *           port       -- specified port
 * OUTPUT  : None.
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 * NOTES   : This api only used by poedrv send msg to poe module
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_POEDRV_PowerDeniedOccurFrequently(UI32_T src_csc_id, UI32_T unit, UI32_T port)
{
    SYSFUN_Msg_T                                        *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                    *syscb_msg_p;
    SYS_CALLBACK_MGR_POEDRV_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_POEDRV_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_POEDRV_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_POEDRV_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_POWER_DENIED_OCCUR_FRENQUENTLY;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_POEDRV_PortFailureStatusChange
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when detection status change
 * INPUT   : src_csc_id -- The csc_id who triggers this event (SYS_MODULE_POEDRV)
 *           unit       -- specified unit
 *           port       -- specified port
 *           status     -- actual status
 * OUTPUT  : None.
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 * NOTES   : This api only used by poedrv send msg to poe module
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_POEDRV_PortFailureStatusChange(UI32_T src_csc_id, UI32_T unit, UI32_T port, UI32_T status)
{
    SYSFUN_Msg_T                      *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T            *syscb_msg_p;
    SYS_CALLBACK_MGR_POEDRV_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_POEDRV_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_POEDRV_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_POEDRV_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_FAILURE_STATUS_CHANGE;
    cbdata_msg_p->unit = unit;
    cbdata_msg_p->port = port;
    cbdata_msg_p->value = status;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}
#endif

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_AnnounceMgmtIPFltChanged
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when Mgmt IP filter changed.
 * INPUT   :
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_AnnounceMgmtIPFltChanged(UI32_T src_csc_id, UI32_T mode)
{
#define CBDATA  SYS_CALLBACK_MGR_AnnounceMgmtIPFltChanged_CBData_T

    enum {SIZE_OF_CBDATA = sizeof(CBDATA)};

    SYSFUN_Msg_T            *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T  *syscb_msg_p;
    CBDATA                  *cbdata_msg_p;
    UI8_T                   ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(SIZE_OF_CBDATA))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(SIZE_OF_CBDATA);
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (CBDATA*)syscb_msg_p->callback_data;

    /* Add your code here
     */
    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_MGMT_IP_FLT_CHANGED;
    cbdata_msg_p->mode = mode;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);

#undef CBDATA
}
#endif /* #if (SYS_CPNT_MGMT_IP_FLT == TRUE) */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_IPCFG_NsmRouteChange
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when detection nsm rib route change
 * INPUT   : src_csc_id     -- The csc_id who triggers this event (NSM)
 *           address_family -- IPv4 or IPv6
 * OUTPUT  : None.
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 * NOTES   : This api only used by nsm send msg to ipcfg module
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_IPCFG_NsmRouteChange(UI32_T src_csc_id, UI32_T address_family)
{
    SYSFUN_Msg_T                      *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T            *syscb_msg_p;
    SYS_CALLBACK_MGR_IPCFG_NsmRouteChange_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_IPCFG_NsmRouteChange_CBData_T)))];


    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_IPCFG_NsmRouteChange_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_IPCFG_NsmRouteChange_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_IPCFG_NSM_ROUTE_CHANGE;
    cbdata_msg_p->address_family = address_family;


    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_SetPortStatusCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when set the port admin status
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           ifindex -- which logical port
 *           status
 *           reason  -- bitmap of SWCTRL_PORT_STATUS_SET_BY_XXX
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_SetPortStatusCallback(
    UI32_T src_csc_id, UI32_T ifindex, BOOL_T status, UI32_T reason)
{
    UI8_T  ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(
        sizeof(SYS_CALLBACK_MGR_LPortStatus_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T *)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_LPortStatus_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_LPortStatus_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_LPortStatus_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_SET_PORT_STATUS;
    cbdata_msg_p->ifindex = ifindex;
    cbdata_msg_p->status_bool = status;
    cbdata_msg_p->status_u32 = reason;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

#if (SYS_CPNT_CFM == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_MGR_CFM_DefectNotify
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Notify port link down.
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      unit
 *      port
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_CFM_DefectNotify(UI32_T src_csc_id,
                                             UI32_T type,
                                             UI32_T mep_id,
                                             UI32_T lport,
                                             UI8_T level,
                                             UI16_T vid,
                                             BOOL_T defected)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T                  *syscb_msg_p;
    SYS_CALLBACK_MGR_CFM_DefectNotify_CBData_T  *cbdata_msg_p;
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_CFM_DefectNotify_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_CFM_DefectNotify_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_CFM_DefectNotify_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_CFM_DEFECT_NOTIFY;
    cbdata_msg_p->lport = lport;
    cbdata_msg_p->mep_id = mep_id;
    cbdata_msg_p->level = level;
    cbdata_msg_p->vid = vid;
    cbdata_msg_p->type = type;
    cbdata_msg_p->defected = defected;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE);
}
#endif /* #if (SYS_CPNT_CFM == TRUE) */

#if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE)
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_ReceiveRaGuardPacketCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When IML received packet for RA Guard, it will notify RA Group with this function.
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      ing_vid        --- ingress vlan id
 *      ing_cos        --- ingress cos value
 *      pkt_length     --- the length of the receive packet
 *      src_lport      --- source lport id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  Callback messages have been sent successfully
 *      FALSE --  Fail to send some of the callback messages
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveRaGuardPacketCallback(UI32_T   src_csc_id,
                                                     L_MM_Mref_Handle_T *mref_handle_p,
                                                     UI8_T    dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                     UI8_T    src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                     UI16_T   ing_vid,
                                                     UI8_T    ing_cos,
                                                     UI8_T    pkt_type,
                                                     UI32_T   pkt_length,
                                                     UI32_T   src_lport)
{
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_RaGuardReceivePacket_CBData_T)))];
    SYSFUN_Msg_T                                    *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                          *syscb_msg_p;
    SYS_CALLBACK_MGR_RaGuardReceivePacket_CBData_T  *cbdata_msg_p;
    UI32_T                                          fail_count = 0;

    SYS_CALLBACK_MGR_REFINE_MREF_REFCOUNT(mref_handle_p, SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_RAGUARD_PACKET);

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_RaGuardReceivePacket_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_RaGuardReceivePacket_CBData_T *)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id   = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_RAGUARD_PACKET;
    cbdata_msg_p->mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    cbdata_msg_p->pkt_length         = pkt_length;
    cbdata_msg_p->ing_vid            = ing_vid;
    cbdata_msg_p->ing_cos            = ing_cos;
    cbdata_msg_p->pkt_type           = pkt_type;
    cbdata_msg_p->src_lport          = src_lport;
    memcpy(cbdata_msg_p->src_mac, src_mac, 6);
    memcpy(cbdata_msg_p->dst_mac, dst_mac, 6);

    if (FALSE == SYS_CALLBACK_MGR_SendMsgWithFailCount(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE, &fail_count))
    {
        while (fail_count>0)
        {
            fail_count--;
            L_MM_Mref_Release(&mref_handle_p);
        };
        return FALSE;
    }

    return TRUE;
}
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE) */


/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_HandleReceiveNdPacket
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When L2MUX received IP packet, it will check if this is nd packet and
 *      other CSC needs in this function
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      ing_vid        --- ingress vlan id
 *      ing_cos        --- ingress cos value
 *      pkt_length     --- the length of the receive packet
 *      src_lport      --- source lport id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  upper layer(IML)need this packet
 *      FALSE --  upper layer(IML)doesn't need this packet
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_HandleReceiveNdPacket(UI32_T   src_csc_id,
                                                L_MM_Mref_Handle_T *mref_handle_p,
                                                UI8_T    dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                UI8_T    src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                UI16_T   ing_vid,
                                                UI16_T   ether_type,
                                                UI32_T   pkt_length,
                                                UI32_T   unit_no,
                                                UI32_T   port_no)
{
    void  *payload_p=NULL;
    SYS_CALLBACK_MGR_Icmp6Hdr_T  *icmp6_hdr_p=NULL;
    UI32_T payload_len=0;
    UI32_T ext_hdr_len=0;
    UI32_T next_hdr_type=0;
    UI32_T ing_lport=0;
    UI32_T avail_size_before_pdu=0;
#if (SYS_CPNT_STACKING == TRUE)
    UI16_T iuc_ethernet_header_len = 0;
    UI16_T isc_header_len = 0;
    UI16_T stacking_header_len = 0;
    UI16_T ethernet_header_len = 0;
#endif /* SYS_CPNT_STACKING */
#if (SYS_CPNT_NDSNP == TRUE)
    BOOL_T is_ndsnp_pkt = FALSE;
#endif /* SYS_CPNT_NDSNP */
#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
    UI32_T ragd_pkt_type=0;
    BOOL_T is_raguard_pkt = FALSE;
#endif /* SYS_CPNT_IPV6_RA_GUARD */

    /* calculate available size before pdu */
    avail_size_before_pdu = L_MM_Mref_GetAvailSzBeforePdu(mref_handle_p);
#if (SYS_CPNT_STACKING == TRUE)
    ICU_GetIUCEthHeaderLen(&iuc_ethernet_header_len);
    ISC_GetISCHeaderLen(&isc_header_len);
    LAN_GetStackingHeaderLen(&stacking_header_len);
    LAN_GetEthHeaderLen(FALSE, &ethernet_header_len);

    if ((avail_size_before_pdu > ethernet_header_len) &&
        (avail_size_before_pdu <= (ethernet_header_len + iuc_ethernet_header_len +
                                isc_header_len + stacking_header_len)))
    {
        avail_size_before_pdu -= (iuc_ethernet_header_len + isc_header_len + stacking_header_len);
    }
#endif /* SYS_CPNT_STACKING */

    /* get pdu */
    payload_p = L_MM_Mref_GetPdu(mref_handle_p, &payload_len);

    if (ether_type == IPV6_FORMAT)
    {
        if (TRUE == SYS_CALLBACK_MGR_GetTotalExtHdrLen(payload_p, &ext_hdr_len, &next_hdr_type))
        {
            if (next_hdr_type == IPPROTO_ICMPV6)
            {
                icmp6_hdr_p = payload_p + sizeof(Ipv6PktFormat_T) + ext_hdr_len;
                switch (icmp6_hdr_p->icmp6_type)
                {
                    case ICMPV6_NDISC_NEIGH_SOL:
                    case ICMPV6_NDISC_NEIGH_ADVT:
                    case ICMPV6_NDISC_ROUTER_SOL:
#if (SYS_CPNT_NDSNP == TRUE)
                        is_ndsnp_pkt = TRUE;
#endif  /* SYS_CPNT_NDSNP */
                        break;
                    case ICMPV6_NDISC_ROUTER_ADVT:
#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
                        ragd_pkt_type = NETCFG_TYPE_RG_PKT_RA;
                        is_raguard_pkt = TRUE;
#endif  /* SYS_CPNT_IPV6_RA_GUARD */
#if (SYS_CPNT_NDSNP == TRUE)
                        is_ndsnp_pkt = TRUE;
#endif  /* SYS_CPNT_NDSNP */
                        break;
                    case ICMPV6_NDISC_REDIRECT:
#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
                        ragd_pkt_type = NETCFG_TYPE_RG_PKT_RR;
                        is_raguard_pkt = TRUE;
#endif  /* SYS_CPNT_IPV6_RA_GUARD */
#if (SYS_CPNT_NDSNP == TRUE)
                        is_ndsnp_pkt = TRUE;
#endif  /* SYS_CPNT_NDSNP */
                        break;
                    default:
                        break;
                 }
             }
         }

    }

    /* Mapping user port ID to logical port ID */
    if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UserPortToLogicalPort(unit_no, port_no, &ing_lport))
    {
        /* this port is an unknown port */
        L_MM_Mref_Release(&mref_handle_p);
        return FALSE;
    }

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
{
    /* if this is RA or Redirect packet, and ingress port is enabled RA guard, drop it */
    if(is_raguard_pkt)
    {
        if(TRUE == NETCFG_POM_ND_RAGUARD_IsEnabled(ing_lport, ragd_pkt_type))
        {
            /* RA guard enabled on ingress port, drop it */
            L_MM_Mref_Release(&mref_handle_p);
            return FALSE;
        }
    }
}
#endif


    /* check if NDSNP needs */
#if (SYS_CPNT_NDSNP == TRUE)
    {
        L_MM_Mref_Handle_T  *tmp_mref_p=NULL;
        UI8_T *src_pkt_p=NULL;
        UI8_T *dst_pkt_p=NULL;
        UI32_T src_len=0;
        UI32_T dst_len=0;
        UI8_T global_status=0;
        if(is_ndsnp_pkt)
        {

            NDSNP_POM_GetGlobalSnoopingStatus(&global_status);
            if(NDSNP_TYPE_GLOBAL_SNOOPING_ENABLE == global_status)
            {
                /* clone packet */
                {
                    /* move the orignal packet pointer to the ethernet head */
                    src_pkt_p = L_MM_Mref_MovePdu(mref_handle_p, (0-avail_size_before_pdu), &src_len);

                    if (NULL == src_pkt_p)
                    {
                        L_MM_Mref_Release(&mref_handle_p);
                        return FALSE;
                    }
                    /* allocate enough buffer for cloned packet */
                    tmp_mref_p = L_MM_AllocateTxBuffer(payload_len,
                        L_MM_USER_ID2(src_csc_id, SYS_CALLBACK_MGR_TRACE_ID_HANDLE_ND_PACKET_CALLBACK));

                    if(NULL == tmp_mref_p)
                    {
                        L_MM_Mref_Release(&mref_handle_p);
                        return FALSE;
                    }
                    /* move pointer to the ethernet head */
                    dst_pkt_p = L_MM_Mref_MovePdu(tmp_mref_p, (0-avail_size_before_pdu), &dst_len);

                    if(NULL == dst_pkt_p)
                    {
                        L_MM_Mref_Release(&mref_handle_p);
                        L_MM_Mref_Release(&tmp_mref_p);
                        return FALSE;
                    }
                    /* copy the whole packet */
                    memcpy(dst_pkt_p, src_pkt_p, src_len);

                    /* clone success,
                     * move the original pdu pointer back to ip header
                     */
                    src_pkt_p = L_MM_Mref_MovePdu(mref_handle_p, avail_size_before_pdu, &src_len);
                    dst_pkt_p = L_MM_Mref_MovePdu(tmp_mref_p, avail_size_before_pdu, &dst_len);
                }

                /* Send duplicated packet to NDSNP_PMGR */
                if(NDSNP_TYPE_ANNOUNCE_PACKET == NDSNP_PMGR_ProcessRxNdPkt(tmp_mref_p,
                                                                            pkt_length,
                                                                            ext_hdr_len,
                                                                            dst_mac,
                                                                            src_mac,
                                                                            (UI32_T)ing_vid,
                                                                            ing_lport))
                {
                    /* announce original packet to upper layer */
                    return TRUE;
                }
                else
                {
                    /* free original mref */
                    L_MM_Mref_Release(&mref_handle_p);
                    return FALSE;
                }


            }
        }
    }
#endif


    return TRUE;
}


/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_HandleSendNdPacket
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When L2MUX received IP packet, it will check if this is nd packet and
 *      other CSC needs in this function
 *
 * INPUT:
 *      src_csc_id     --- The csc_id(SYS_MODULE_LAN defined in sys_module.h) who triggers this event
 *      mref_handle_p  --- The memory reference address
 *      dst_mac        --- destination mac address
 *      src_mac        --- source mac address
 *      ing_vid        --- ingress vlan id
 *      ing_cos        --- ingress cos value
 *      pkt_length     --- the length of the receive packet
 *      src_lport      --- source lport id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  --  upper layer(IML)need this packet
 *      FALSE --  upper layer(IML)doesn't need this packet
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_HandleSendNdPacket(UI32_T   src_csc_id,
                                           L_MM_Mref_Handle_T *mref_handle_p,
                                           UI8_T    dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                           UI8_T    src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                           UI16_T   ether_type,
                                           UI32_T   pkt_length,
                                           UI16_T   egress_vid)
{
    void  *payload_p=NULL;
    SYS_CALLBACK_MGR_Icmp6Hdr_T  *icmp6_hdr_p=NULL;
    UI32_T payload_len=0;
    UI32_T ext_hdr_len=0;
    UI32_T next_hdr_type=0;
    UI32_T avail_size_before_pdu=0;
#if (SYS_CPNT_STACKING == TRUE)
    UI16_T iuc_ethernet_header_len = 0;
    UI16_T isc_header_len = 0;
    UI16_T stacking_header_len = 0;
    UI16_T ethernet_header_len = 0;
#endif /* SYS_CPNT_STACKING */

    /* calculate available size before pdu */
    avail_size_before_pdu = L_MM_Mref_GetAvailSzBeforePdu(mref_handle_p);
#if (SYS_CPNT_STACKING == TRUE)
    ICU_GetIUCEthHeaderLen(&iuc_ethernet_header_len);
    ISC_GetISCHeaderLen(&isc_header_len);
    LAN_GetStackingHeaderLen(&stacking_header_len);
    LAN_GetEthHeaderLen(FALSE, &ethernet_header_len);

    if ((avail_size_before_pdu > ethernet_header_len) &&
        (avail_size_before_pdu <= (ethernet_header_len + iuc_ethernet_header_len +
                                isc_header_len + stacking_header_len)))
    {
        avail_size_before_pdu -= (iuc_ethernet_header_len + isc_header_len + stacking_header_len);
    }
#endif /* SYS_CPNT_STACKING */

    /* get pdu */
    payload_p = L_MM_Mref_GetPdu(mref_handle_p, &payload_len);

    if (ether_type == IPV6_FORMAT)
    {
        if (TRUE == SYS_CALLBACK_MGR_GetTotalExtHdrLen(payload_p, &ext_hdr_len, &next_hdr_type))
        {
            if (next_hdr_type == IPPROTO_ICMPV6)
            {
                icmp6_hdr_p = payload_p + sizeof(SYS_CALLBACK_MGR_Icmp6Hdr_T) + ext_hdr_len;
                switch (icmp6_hdr_p->icmp6_type)
                {


                    case ICMPV6_NDISC_ROUTER_ADVT:
                    {
#if (SYS_CPNT_NDSNP == TRUE)
                        UI8_T global_status=0;
                        UI8_T vlan_status=0;

                        NDSNP_POM_GetGlobalSnoopingStatus(&global_status);
                        NDSNP_POM_GetVlanSnoopingStatus((UI32_T)egress_vid, &vlan_status);
                        if((NDSNP_TYPE_GLOBAL_SNOOPING_ENABLE == global_status)&&
                           (NDSNP_TYPE_VLAN_SNOOPING_ENABLE == vlan_status))
                        {
                            if(NDSNP_TYPE_OK != NDSNP_PMGR_ProcessTxNdPkt(
                                                    mref_handle_p,
                                                    pkt_length,
                                                    ext_hdr_len,
                                                    dst_mac,
                                                    src_mac))
                            {
                                return FALSE;
                            }
                        }
#endif
                        }
                        break;


                    default:
                        break;
                 }
             }
         }

    }
    return TRUE;
}

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_BackDoorMenu
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      entry of backdoor
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
void SYS_CALLBACK_MGR_BackDoorMenu(void)
{
    static struct {
        UI32_T debug_flag;
        char *str;
    } menu_id_2_dbg_flag[] = {
        { SYS_CALLBACK_OM_DBG_IPC_SENT,         "dbg_show_ipc_sent" },
        { SYS_CALLBACK_OM_DBG_IPC_FAILED,       "dbg_show_ipc_fail" },
        { SYS_CALLBACK_OM_DBG_IPC_DETALL,       "dbg_show_detail" },
        { SYS_CALLBACK_OM_DBG_IPC_DUMP,         "dbg_dump_ipc_msg" },
        { SYS_CALLBACK_OM_DBG_IPC_COUNTING,     "dbg_ipc_counting" },
    };

    SYS_CALLBACK_OM_Backdoor_T *bddb_p = SYS_CALLBACK_OM_GetBackdoorDb();
    char buf[16];
    int i, ch;
    unsigned long ul;

    while (1)
    {
        for (i = 0; i < sizeof(menu_id_2_dbg_flag)/sizeof(*menu_id_2_dbg_flag); i++)
        {
            BACKDOOR_MGR_Printf("\n %d. %s: %d", i, menu_id_2_dbg_flag[i].str, SYS_CALLBACK_OM_IsDebugFlagOn(menu_id_2_dbg_flag[i].debug_flag, TRUE));
        }
        BACKDOOR_MGR_Print("\n--");
        BACKDOOR_MGR_Printf("\n c. dbg_ipc_count: %lu", (unsigned long)bddb_p->dbg_ipc_count);
        BACKDOOR_MGR_Print("\n e. dbg_event_id: "); if (bddb_p->dbg_event_id == 0xffffffff) BACKDOOR_MGR_Print("*"); else BACKDOOR_MGR_Printf("%lu", (unsigned long)bddb_p->dbg_event_id);
        BACKDOOR_MGR_Print("\n s. dbg_src_id: "); if (bddb_p->dbg_src_id == 0xffffffff) BACKDOOR_MGR_Print("*"); else BACKDOOR_MGR_Printf("%lu", (unsigned long)bddb_p->dbg_src_id);
        BACKDOOR_MGR_Print("\n d. dbg_dst_id: "); if (bddb_p->dbg_dst_id == 0xffffffff) BACKDOOR_MGR_Print("*"); else BACKDOOR_MGR_Printf("%lu", (unsigned long)bddb_p->dbg_dst_id);
        BACKDOOR_MGR_Print("\n--");
        BACKDOOR_MGR_Print("\n q. exit");
        BACKDOOR_MGR_Print("\n select = ");
        ch = BACKDOOR_MGR_GetChar();
        BACKDOOR_MGR_Printf("%c\n", (isprint(ch) ? ch : '?'));

        if (isdigit(ch))
        {
            i = ch - '0';

            if (i >= 0 && i < sizeof(menu_id_2_dbg_flag)/sizeof(*menu_id_2_dbg_flag))
            {
                SYS_CALLBACK_OM_SetDebugFlag(menu_id_2_dbg_flag[i].debug_flag,
                    !SYS_CALLBACK_OM_IsDebugFlagOn(menu_id_2_dbg_flag[i].debug_flag, TRUE));
                continue;
            }
        }

        switch (ch)
        {
            case 'c':
                bddb_p->dbg_ipc_count = 0;
                break;

            case 'e':
                BACKDOOR_MGR_Print("dbg_event_id: ");
                BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
                if (sscanf(buf, "%lu", &ul) < 1)
                    bddb_p->dbg_event_id = 0xffffffff;
                else
                    bddb_p->dbg_event_id = ul;
                BACKDOOR_MGR_Print("\n");
                break;

            case 's':
                BACKDOOR_MGR_Print("dbg_src_id: ");
                BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
                if (sscanf(buf, "%lu", &ul) < 1)
                    bddb_p->dbg_src_id = 0xffffffff;
                else
                    bddb_p->dbg_src_id = ul;
                BACKDOOR_MGR_Print("\n");
                break;

            case 'd':
                BACKDOOR_MGR_Print("dbg_dst_id: ");
                BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
                if (sscanf(buf, "%lu", &ul) < 1)
                    bddb_p->dbg_dst_id = 0xffffffff;
                else
                    bddb_p->dbg_dst_id = ul;
                BACKDOOR_MGR_Print("\n");
                break;

            case 'q':
                return;
        }
    }
}

static BOOL_T SYS_CALLBACK_MGR_GetExtHdrLen(UI8_T *payload_p, UI8_T ext_hdr_type, UI32_T *ext_hdr_len_p)
{
    switch (ext_hdr_type)
    {
        case IPV6_EXT_HDR_HOP_BY_HOP:
            *ext_hdr_len_p = (*(payload_p+1)+1)<<3;
            break;
        case IPV6_EXT_HDR_DESTINATION:
            *ext_hdr_len_p = (*(payload_p+1)+1)<<3;
            break;
        case IPV6_EXT_HDR_ROUTING:
            *ext_hdr_len_p = (*(payload_p+1)+1)<<3;
            break;
        case IPV6_EXT_HDR_FRAGMENT:
            *ext_hdr_len_p = 8;
            break;
        case IPV6_EXT_HDR_AUTHENTICATION:
            *ext_hdr_len_p = (*(payload_p+1)+2)<<2;
            break;
        case IPV6_EXT_HDR_SECURITY: /* because payload is encrypted */
        default: /* not an IPV6 extension header */
            return FALSE;
    }

    return TRUE;
}

static BOOL_T SYS_CALLBACK_MGR_GetTotalExtHdrLen(UI8_T *payload_p, UI32_T *total_ext_hdr_len_p, UI32_T *next_hdr_type_p)
{
    Ipv6PktFormat_T *ipv6_pkt_p   = (Ipv6PktFormat_T*)payload_p;

    if (ipv6_pkt_p->next_hdr != IPPROTO_ICMPV6 &&
        ipv6_pkt_p->next_hdr != IPPROTO_TCP &&
        ipv6_pkt_p->next_hdr != IPPROTO_UDP &&
        ipv6_pkt_p->next_hdr != IPPROTO_NONE)
    {
        UI8_T   *ipv6_opt_hdr_p;
        UI32_T  ext_hdr_len=0;
        UI8_T   next_hdr_type=0;

        next_hdr_type  = ipv6_pkt_p->next_hdr;
        ipv6_opt_hdr_p = ipv6_pkt_p->pay_load;
        *total_ext_hdr_len_p = 0;
        while (SYS_CALLBACK_MGR_GetExtHdrLen(ipv6_opt_hdr_p, next_hdr_type, &ext_hdr_len) == TRUE)
        {
            next_hdr_type = *ipv6_opt_hdr_p;
            ipv6_opt_hdr_p = ipv6_opt_hdr_p + ext_hdr_len;
            *total_ext_hdr_len_p += ext_hdr_len;
        }
        if (next_hdr_type == IPPROTO_ICMPV6 ||
            next_hdr_type == IPPROTO_TCP ||
            next_hdr_type == IPPROTO_UDP ||
            next_hdr_type == IPPROTO_NONE)
        {
            *next_hdr_type_p = next_hdr_type;
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else /* no extension header */
    {
        *next_hdr_type_p = ipv6_pkt_p->next_hdr;
        *total_ext_hdr_len_p = 0;
    }

    return TRUE;
}

static void SYS_CALLBACK_MGR_Backdoor_DebugIpc(UI32_T src_csc_id, UI32_T dest_msgq_key, SYSFUN_Msg_T *sysfun_msg_p, UI32_T sysfun_ret)
{
    SYS_CALLBACK_OM_Backdoor_T *bddb_p = SYS_CALLBACK_OM_GetBackdoorDb();
    SYS_CALLBACK_MGR_Msg_T *syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T *)sysfun_msg_p->msg_buf;
    SYSFUN_MsgQ_T msgq_handle;
    UI32_T task_id;
    char mname[L_MNAME_MAX_NAME_LEN+1];
    char tname[16];
    BOOL_T show_dbg_msg;
    BOOL_T ipc_counting;

    show_dbg_msg =
        ((sysfun_ret == SYSFUN_OK && SYS_CALLBACK_OM_IsDebugFlagOn(SYS_CALLBACK_OM_DBG_IPC_SENT, TRUE)) ||
         (sysfun_ret != SYSFUN_OK && SYS_CALLBACK_OM_IsDebugFlagOn(SYS_CALLBACK_OM_DBG_IPC_FAILED, TRUE)));

    ipc_counting =
        SYS_CALLBACK_OM_IsDebugFlagOn(SYS_CALLBACK_OM_DBG_IPC_COUNTING, TRUE);

    if (!show_dbg_msg && !ipc_counting)
    {
        return;
    }

    /* get src module name
     */
    L_MNAME_GetModuleName(src_csc_id, (void *)mname);

    /* get dst task id/name
     */
    SYSFUN_GetMsgQ(dest_msgq_key, SYSFUN_MSGQ_BIDIRECTIONAL, &msgq_handle);
    task_id = SYSFUN_GetMsgQOwner(msgq_handle);
    if (SYSFUN_OK != SYSFUN_TaskIDToName(task_id, tname, sizeof(tname)))
        strncpy(tname, "Unknown", sizeof(tname)-1);
    tname[sizeof(tname)-1] = 0;

    /*  check filter
     */
    if ((bddb_p->dbg_event_id != 0xffffffff &&
         bddb_p->dbg_event_id != syscb_msg_p->callback_event_id) ||
        (bddb_p->dbg_src_id != 0xffffffff &&
         bddb_p->dbg_src_id != src_csc_id) ||
        (bddb_p->dbg_dst_id != 0xffffffff &&
         bddb_p->dbg_dst_id != task_id))
    {
        return;
    }

    /* ipc counting
     */
    if (ipc_counting)
    {
        bddb_p->dbg_ipc_count++;
    }

    /* show debug message
     */
    if (show_dbg_msg)
    {
        BACKDOOR_MGR_Printf(
            "SYS_CALLBACK: event:%lu src:%s(%lu) dest:%s(%lu) ret:%lu\n",
            (unsigned long)syscb_msg_p->callback_event_id,
            mname,
            (unsigned long)src_csc_id,
            tname,
            (unsigned long)task_id,
            (unsigned long)sysfun_ret);

        /* show detail
         */
        if (SYS_CALLBACK_OM_IsDebugFlagOn(SYS_CALLBACK_OM_DBG_IPC_DETALL, TRUE))
        {
            SYS_CALLBACK_MGR_Backdoor_DebugSyscbMsg(syscb_msg_p);
        }

        /* dump ipc msg
         */
        if (SYS_CALLBACK_OM_IsDebugFlagOn(SYS_CALLBACK_OM_DBG_IPC_DUMP, TRUE))
        {
            BACKDOOR_MGR_DumpHex("DUMP syscb_msg_p", sizeof(*syscb_msg_p), syscb_msg_p);
            if (sysfun_msg_p->msg_size > sizeof(*syscb_msg_p))
            {
                BACKDOOR_MGR_DumpHex("DUMP syscb_msg_p->callback_data", sysfun_msg_p->msg_size - sizeof(*syscb_msg_p), syscb_msg_p->callback_data);
            }
        } /* end of if (show_detail) */
    } /* end of if (show_dbg_msg) */
}

static void SYS_CALLBACK_MGR_Backdoor_DebugSyscbMsg(SYS_CALLBACK_MGR_Msg_T *syscb_msg_p)
{
    switch (syscb_msg_p->callback_event_id)
    {
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_L2MUX_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LACP_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_OAM_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_OAM_LBK_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LOOPBACK_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_DOT1X_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_STA_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_GVRP_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LLDP_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_SFLOW_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LBD_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_UDLD_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_IP_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_MLAG_PACKET:
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_OF_PACKET:
        {
            SYS_CALLBACK_MGR_LanReceivePacket_CBData_T *cb_data_p = (SYS_CALLBACK_MGR_LanReceivePacket_CBData_T *)syscb_msg_p->callback_data;;

            BACKDOOR_MGR_Printf(
                " dmac:%02x-%02x-%02x-%02x-%02x-%02x"
                " smac:%02x-%02x-%02x-%02x-%02x-%02x\n"
                " taginfo:%02x-%02x"
                " etype:%02x-%02x"
                " pkt_len:%lu\n"
                " uport:%lu/%lu"
                " pkt_type:%lu\n",
                cb_data_p->dst_mac[0],cb_data_p->dst_mac[1],cb_data_p->dst_mac[2],cb_data_p->dst_mac[3],cb_data_p->dst_mac[4],cb_data_p->dst_mac[5],
                cb_data_p->src_mac[0],cb_data_p->src_mac[1],cb_data_p->src_mac[2],cb_data_p->src_mac[3],cb_data_p->src_mac[4],cb_data_p->src_mac[5],
                cb_data_p->tag_info >> 8, cb_data_p->tag_info & 0xff,
                cb_data_p->type >> 8, cb_data_p->type & 0xff,
                (unsigned long)cb_data_p->pkt_length,
                (unsigned long)cb_data_p->src_unit,
                (unsigned long)cb_data_p->src_port,
                (unsigned long)cb_data_p->packet_class);

            break;
        }

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_AUTHENTICATE_PACKET:
        {
            SYS_CALLBACK_MGR_AuthenticatePacket_CBData_T *cb_data_p = (SYS_CALLBACK_MGR_AuthenticatePacket_CBData_T *)syscb_msg_p->callback_data;

            BACKDOOR_MGR_Printf(
                " auth_result: %d"
                " dmac:%02x-%02x-%02x-%02x-%02x-%02x"
                " smac:%02x-%02x-%02x-%02x-%02x-%02x\n"
                " taginfo:%02x-%02x"
                " etype:%02x-%02x"
                " pkt_len:%lu\n"
                " uport:%lu/%lu"
                " pkt_type:%lu\n",
                cb_data_p->auth_result,
                cb_data_p->lan_cbdata.dst_mac[0],cb_data_p->lan_cbdata.dst_mac[1],cb_data_p->lan_cbdata.dst_mac[2],cb_data_p->lan_cbdata.dst_mac[3],cb_data_p->lan_cbdata.dst_mac[4],cb_data_p->lan_cbdata.dst_mac[5],
                cb_data_p->lan_cbdata.src_mac[0],cb_data_p->lan_cbdata.src_mac[1],cb_data_p->lan_cbdata.src_mac[2],cb_data_p->lan_cbdata.src_mac[3],cb_data_p->lan_cbdata.src_mac[4],cb_data_p->lan_cbdata.src_mac[5],
                cb_data_p->lan_cbdata.tag_info >> 8, cb_data_p->lan_cbdata.tag_info & 0xff,
                cb_data_p->lan_cbdata.type >> 8, cb_data_p->lan_cbdata.type & 0xff,
                (unsigned long)cb_data_p->lan_cbdata.pkt_length,
                (unsigned long)cb_data_p->lan_cbdata.src_unit,
                (unsigned long)cb_data_p->lan_cbdata.src_port,
                (unsigned long)cb_data_p->lan_cbdata.packet_class);

            break;
        }
        default:
            ;
    } /* end of switch (callback_event_id) */
}

#if(SYS_CPNT_DCBX == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_ReceiveDcbxEtsTlvCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Notify other CSC when LLDP receives DCBX ETS TLV
 * INPUT   : src_csc_id          -- The csc_id who triggers this event (SYS_MODULE_LLDP)
 *           lport                --
 *           rem_recommend_rcvd                --
 *           rem_willing          --
 *           rem_cbs        --
 *           rem_max_tc      --
 *           rem_config_pri_assign_table  --
 *           rem_config_tc_bandwidth_table --
 *           rem_config_tsa_assign_table --
 *           rem_recommend_pri_assign_table  --
 *           rem_recommend_tc_bandwidth_table --
 *           rem_recommend_tsa_assign_table --
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveDcbxEtsTlvCallback(UI32_T   src_csc_id,
                                                    UI32_T   lport,
                                                    BOOL_T  is_delete,
                                                    BOOL_T  rem_recommend_rcvd,
                                                    BOOL_T  rem_willing,
                                                    BOOL_T  rem_cbs,
                                                    UI8_T  rem_max_tc,
                                                    UI8_T  *rem_config_pri_assign_table,
                                                    UI8_T   *rem_config_tc_bandwidth_table,
                                                    UI8_T   *rem_config_tsa_assign_table,
                                                    UI8_T  *rem_recommend_pri_assign_table,
                                                    UI8_T   *rem_recommend_tc_bandwidth_table,
                                                    UI8_T   *rem_recommend_tsa_assign_table)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T *syscb_msg_p;
    SYS_CALLBACK_MGR_DcbxEtsTlvReceived_CBData_T *cbdata_msg_p;
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_DcbxEtsTlvReceived_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_DcbxEtsTlvReceived_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p = (SYS_CALLBACK_MGR_DcbxEtsTlvReceived_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_ETS_TLV;
    cbdata_msg_p->lport = lport;
    cbdata_msg_p->is_delete = is_delete;
    cbdata_msg_p->rem_recommend_rcvd = rem_recommend_rcvd;
    cbdata_msg_p->rem_willing = rem_willing;
    cbdata_msg_p->rem_cbs = rem_cbs;
    cbdata_msg_p->rem_max_tc = rem_max_tc;
    memcpy(&cbdata_msg_p->rem_config_pri_assign_table[0], rem_config_pri_assign_table, 4);
    memcpy(&cbdata_msg_p->rem_config_tc_bandwidth_table[0], rem_config_tc_bandwidth_table, 8);
    memcpy(&cbdata_msg_p->rem_config_tsa_assign_table[0], rem_config_tsa_assign_table, 8);
    memcpy(&cbdata_msg_p->rem_recommend_pri_assign_table[0], rem_recommend_pri_assign_table, 4);
    memcpy(&cbdata_msg_p->rem_recommend_tc_bandwidth_table[0], rem_recommend_tc_bandwidth_table, 8);
    memcpy(&cbdata_msg_p->rem_recommend_tsa_assign_table[0], rem_recommend_tsa_assign_table, 8);

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_ReceiveDcbxPfcTlvCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Notify other CSC when LLDP receives DCBX PFC TLV
 * INPUT   : src_csc_id          -- The csc_id who triggers this event (SYS_MODULE_LLDP)
 *           lport                --
 *           rem_mac                --
 *           rem_willing          --
 *           rem_mbc        --
 *           rem_pfc_cap      --
 *           rem_pfc_enable  --
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_ReceiveDcbxPfcTlvCallback(UI32_T   src_csc_id,
                                                    UI32_T   lport,
                                                    BOOL_T  is_delete,
                                                     UI8_T   *rem_mac,
                                                    BOOL_T  rem_willing,
                                                    BOOL_T  rem_mbc,
                                                    UI8_T  rem_pfc_cap,
                                                    UI8_T  rem_pfc_enable)
{
    SYSFUN_Msg_T *sysfun_msg_p;
    SYS_CALLBACK_MGR_Msg_T *syscb_msg_p = NULL;
    SYS_CALLBACK_MGR_DcbxPfcTlvReceived_CBData_T *cbdata_msg_p = NULL;
    UI8_T ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_DcbxPfcTlvReceived_CBData_T)))];

    sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_DcbxPfcTlvReceived_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p = (SYS_CALLBACK_MGR_DcbxPfcTlvReceived_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_PFC_TLV;
    cbdata_msg_p->lport = lport;
    cbdata_msg_p->is_delete = is_delete;
    memcpy(cbdata_msg_p->rem_mac, rem_mac, 6);
    cbdata_msg_p->rem_willing = rem_willing;
    cbdata_msg_p->rem_mbc = rem_mbc;
    cbdata_msg_p->rem_pfc_cap = rem_pfc_cap;
    cbdata_msg_p->rem_pfc_enable = rem_pfc_enable;
    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_EtsConfigChangedCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : The registered function would be called when a port's ets config has
 *           been changed.
 *
 * INPUT   : src_csc_id -- The csc_id who triggers this event
 *           lport      -- specify the lport's ets config has been changed
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_EtsConfigChangedCallback(
    UI32_T  src_csc_id,
    UI32_T  lport)
{
    SYS_CALLBACK_MGR_FUNC_BEG(
        SYS_CALLBACK_MGR_EtsPfcCfgChanged_CBData_T,
        SYS_CALLBACK_MGR_CALLBACKEVENTID_ETS_CFG_CHANGED);

    cbdata_msg_p->lport = lport;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_PfcConfigChangedCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : The registered function would be called when a port's pfc config has
 *           been changed.
 *
 * INPUT   : src_csc_id -- The csc_id who triggers this event
 *           lport      -- specify the lport's pfc config has been changed
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_PfcConfigChangedCallback(
    UI32_T  src_csc_id,
    UI32_T  lport)
{
    SYS_CALLBACK_MGR_FUNC_BEG(
        SYS_CALLBACK_MGR_EtsPfcCfgChanged_CBData_T,
        SYS_CALLBACK_MGR_CALLBACKEVENTID_PFC_CFG_CHANGED);

    cbdata_msg_p->lport = lport;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, FALSE);
}
#endif

#if (SYS_CPNT_PBR == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_HostRouteChanged
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when host route changed
 * INPUT   : src_csc_id    -- The csc_id who triggers this event
 *           addr          -- host route address
 *           is_unresolved -- is host route unresolved
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_HostRouteChanged(UI32_T src_csc_id, L_INET_AddrIp_T *addr_p, BOOL_T is_unresolved)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_HostRouteChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T             *syscb_msg_p;
    SYS_CALLBACK_MGR_HostRouteChanged_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_HostRouteChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_HostRouteChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_HOST_ROUTE_CHANGED;
    cbdata_msg_p->addr = *addr_p;
    cbdata_msg_p->is_unresolved = is_unresolved;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_AclChanged
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when acl changed
 * INPUT   :  src_csc_id -- The csc_id who triggers this event (L4)
 *            acl_index  -- acl index
 *            acl_name_p -- acl name
 *            type       -- change type: add/delete/modify
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_AclChanged(UI32_T src_csc_id, UI32_T acl_index, char *acl_name_p, UI8_T type)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AclChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T             *syscb_msg_p;
    SYS_CALLBACK_MGR_AclChanged_CBData_T  *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_AclChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_AclChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_ACL_CHANGED;
    cbdata_msg_p->acl_index = acl_index;
    cbdata_msg_p->type = type;
    strncpy(cbdata_msg_p->acl_name, acl_name_p, SYS_ADPT_MAX_ACCESS_LIST_NAME_LENGTH);

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_RouteMapChanged
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when route map changed (added/modified/deleted)
 * INPUT   : src_csc_id  -- The csc_id who triggers this event
 *           rmap_name_p -- Route-map name
 *           seq_num     -- sequence number
 *           is_deleted  -- whether it is deleted
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   :
 *           1. seq_num = 0 means all sequence numbers, only used in is_deleleted = TRUE
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_RouteMapChanged(UI32_T src_csc_id, char *rmap_name, UI32_T seq_num, BOOL_T is_deleted)
{
    UI8_T        ipcmsg_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_RoutemapChanged_CBData_T)))];
    SYSFUN_Msg_T *sysfun_msg_p = (SYSFUN_Msg_T*)ipcmsg_buf;
    SYS_CALLBACK_MGR_Msg_T                *syscb_msg_p;
    SYS_CALLBACK_MGR_RoutemapChanged_CBData_T *cbdata_msg_p;

    sysfun_msg_p->msg_size = SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_RoutemapChanged_CBData_T));
    syscb_msg_p = (SYS_CALLBACK_MGR_Msg_T*)sysfun_msg_p->msg_buf;
    cbdata_msg_p= (SYS_CALLBACK_MGR_RoutemapChanged_CBData_T*)syscb_msg_p->callback_data;

    syscb_msg_p->callback_event_id = SYS_CALLBACK_MGR_CALLBACKEVENTID_ROUTEMAP_CHANGED;
    strncpy(cbdata_msg_p->rmap_name, rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH);
    cbdata_msg_p->seq_num = seq_num;
    cbdata_msg_p->is_deleted = is_deleted;

    return SYS_CALLBACK_MGR_SendMsg(src_csc_id, syscb_msg_p->callback_event_id, sysfun_msg_p, TRUE);
}
#endif

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
static BOOL_T SYS_CALLBACK_MGR_NotifyCmgrGroup()
{
    UI32_T  task_id;

    task_id = SYS_CALLBACK_OM_GetCmgrThreadId();
    if (task_id)
    {
        SYSFUN_SendEvent(task_id, CMGR_EVENT_CALLBACK);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#endif
