/* MODULE NAME:  sys_callback_mgr.h
 * PURPOSE:
 *  System callback manager is responsible for handling all callbacks across
 *  csc groups.
 *
 * NOTES:
 *
 * HISTORY
 *    6/5/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef SYS_CALLBACK_MGR_H
#define SYS_CALLBACK_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "sysrsc_mgr.h"
#include "sys_callback_om.h"
#include "l_mm.h"
#include "leaf_ieeelldp.h"
#include "isc.h"
#include "l_inet.h"
#include "swdrv_type.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* definitions of callback even id
 */
enum
{
    SYS_CALLBACK_MGR_CALLBACKEVENTID_INTRUSION_MAC = 0,                /* AMTR */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_MOVE,                        /* AMTR */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_SECURITY_PORT_MOVE,               /* AMTR */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_AUTO_LEARN,                       /* AMTR */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_TABLE_DELETE_BY_PORT,         /* AMTR */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_TABLE_DELETE_BY_VID,          /* AMTR */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_TABLE_DELETE_BY_VID_AND_PORT, /* AMTR */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_TABLE_DELETE_BY_LIFE_TIME,    /* AMTR */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_MLAG_MAC_UPDATE,                  /* AMTR */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_OVS_MAC_UPDATE,                   /* AMTR */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_ADDR_UPDATE,                  /* AMTR */

    SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_CREATE,                          /* VLAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_CREATE_FOR_GVRP,                 /* VLAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_DESTROY,                         /* VLAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_DESTROY_FOR_GVRP,                /* VLAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_L3VLAN_DESTROY ,                      /* VLAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_ADD,                      /* VLAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_ADD_FOR_GVRP,             /* VLAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE,                   /* VLAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE_FOR_GVRP,          /* VLAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_PORT_MODE,                       /* VLAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_ADD_FIRST_TRUNK_MEMBER,   /* VLAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_ADD_TRUNK_MEMBER,         /* VLAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_DELETE_TRUNK_MEMBER,      /* VLAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_DELETE_LAST_TRUNK_MEMBER, /* VLAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE_BY_TRUNK,          /* VLAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_PVID_CHANGE,                          /* VLAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_IF_OPER_STATUS_CHANGED,               /* VLAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_L3IF_OPER_STATUS_CHANGED,             /* VLAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_ADD_FIRST_TRUNK_MEMBER,          /* VLAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_ADD_TRUNK_MEMBER,                /* VLAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_DELETE_TRUNK_MEMBER,             /* VLAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_DELETE_LAST_TRUNK_MEMBER,        /* VLAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_NAME_CHANGED,                    /* VLAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_PROTOVLAN_GID_BINDING_CHANGED,        /* VLAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_TAG_CHANGED,              /* VLAN */
#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
    SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_LIST,
#endif

    SYS_CALLBACK_MGR_CALLBACKEVENTID_LPORT_ENTER_FORWARDING, /* XSTP */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_LPORT_LEAVE_FORWARDING, /* XSTP */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_LPORT_CHANGE_STATE,     /* XSTP */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_STP_CHANGE_VERSION,     /* XSTP */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_XSTP_LPORT_TC_CHANGE,   /*add by Tony.lei */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_NEW_MAC_ADDRESS, /* AMTRDRV */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_AGING_OUT,   /* AMTRDRV */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_SECURITY_CHECK,  /* AMTRDRV */

    SYS_CALLBACK_MGR_CALLBACKEVENTID_UPDATE_LOCAL_NMTRDRV_STATS,  /* NMTRDRV */

    SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_OPER_UP,                              /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_NOT_OPER_UP,                          /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_ADMIN_ENABLE,                         /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_ADMIN_DISABLE,                        /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_SPEED_DUPLEX,                         /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD_1ST,                      /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD,                          /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE,                       /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE_LST,                   /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_PORT_OPER_UP,                 /* SWCTRL obsolete by TRUNK_MEMBER_ACTIVE */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_PORT_NOT_OPER_UP,             /* SWCTRL obsolete by TRUNK_MEMBER_INACTIVE */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ACTIVE,                       /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_INACTIVE,                     /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_SPEED_DUPLEX,                        /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_LINK_UP,                             /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_LINK_DOWN,                           /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_ADMIN_ENABLE,                        /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_ADMIN_DISABLE,                       /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_STATUS_CHANGED_PASSIVELY,             /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_LINK_DOWN,                            /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_LINK_UP,                              /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_FAST_LINK_UP,                        /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_FAST_LINK_DOWN,                      /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_ADMIN_DISABLE_BEFORE,                 /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_LACP_EFFECTIVE_OPER_STATUS_CHANGED,  /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_DOT1X_EFFECTIVE_OPER_STATUS_CHANGED, /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_EFFECTIVE_OPER_STATUS_CHANGED,        /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_FORWARDING_UPORT_ADD_TO_TRUNK,             /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_FORWARDING_TRUNK_MEMBER_DELETE,            /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_FORWARDING_TRUNK_MEMBER_TO_NON_FORWARDING, /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_LPORT_TYPE_CHANGED,                        /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_TYPE_CHANGED,                        /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_IF_MAU_CHANGED,                            /* SWCTRL */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_LEARNING_STATUS_CHANGED,              /* SWCTRL */

    SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_LINK_UP,      /* SWDRV */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_LINK_DOWN,    /* SWDRV */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_TYPE_CHANGED, /* SWDRV */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_SPEED_DUPLEX, /* SWDRV */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_FLOW_CTRL,    /* SWDRV */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_HOT_SWAP_INSERT,   /* SWDRV */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_HOT_SWAP_REMOVE,   /* SWDRV */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_SFP_PRESENT,  /* SWDRV */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_SFP_INFO,     /* SWDRV */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_SFP_DDM_INFO, /* SWDRV */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_SFP_DDM_INFO_MEASURED,/* SWDRV */
#if (SYS_CPNT_CFM == TRUE)
    SYS_CALLBACK_MGR_CALLBACKEVENTID_CFM_DEFECT_NOTIFY,      /* CFM*/
#endif
    SYS_CALLBACK_MGR_CALLBACKEVENTID_IGMPSNP_STATUS_CHANGED, /* IGMPSNP */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_ROUTER_PORT_ADD,        /* IGMPSNP */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_ROUTER_PORT_DELETE,     /* IGMPSNP */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_GROUP_MEMBER_ADD,       /* IGMPSNP */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_GROUP_MEMBER_DELETE,    /* IGMPSNP */

    SYS_CALLBACK_MGR_CALLBACKEVENTID_POWER_STATUS_CHANGED,      /* SYSMGMT */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_FAN_STATUS_CHANGED,        /* SYSMGMT */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_THERMAL_STATUS_CHANGED,    /* SYSMGMT */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_XFP_MODULE_STATUS_CHANGED, /* SYSMGMT */

    SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_POWER_STATUS_CHANGED,             /* SYSDRV */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_POWER_TYPE_CHANGED,               /* SYSDRV */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_ALARM_INPUT_STATUS_CHANGED,       /* SYSDRV */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_MAJOR_ALARM_OUTPUT_STATUS_CHANGED,/* SYSDRV */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_MINOR_ALARM_OUTPUT_STATUS_CHANGED,/* SYSDRV */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_FAN_STATUS_CHANGED,               /* SYSDRV */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_FAN_SPEED_CHANGED,                /* SYSDRV */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_THERMAL_STATUS_CHANGED,           /* SYSDRV */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_XFP_MODULE_STATUS_CHANGED,        /* SYSDRV */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_SYSDRV_XENPAK_STATUS_CHANGED,            /* SYSDRV */

    SYS_CALLBACK_MGR_CALLBACKEVENTID_ADD_STATIC_TRUNK_MEMBER, /* TRK */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_DEL_STATIC_TRUNK_MEMBER, /* TRK */

    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_IP_PACKET,      /* IML */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_L2MUX_PACKET,   /* LAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_NA_AND_SECURITY_CHECK,  /* LAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LACP_PACKET,    /* LAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_OAM_PACKET,     /* LAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_OAM_LBK_PACKET, /* LAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LOOPBACK_PACKET,/* LAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_DOT1X_PACKET,   /* LAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_SFLOW_PACKET,   /* LAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LBD_PACKET,     /* LAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_AUTHENTICATE_PACKET,    /* LAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_MLAG_PACKET,    /* LAN */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_AUTHENTICATION_DISPATCH,/* LAN */

    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_STKTPLG_PACKET, /* ISC */

    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_STA_PACKET,    /* L2_MUX */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_GVRP_PACKET,   /* L2_MUX */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LLDP_PACKET,   /* L2_MUX */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_ESMC_PACKET,   /* L2_MUX */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_IGMPSNP_PACKET,/* L2_MUX */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_MLDSNP_PACKET, /* L2_MUX */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_CFM_PACKET,    /* L2_MUX */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_ELPS_PACKET,    /* L2_MUX */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_PPPOED_PACKET, /* L2_MUX */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_CLUSTER_PACKET, /* L2_MUX */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_OF_PACKET,      /* L2_MUX */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_PTP_PACKET,    /* L2_MUX */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RX_SNOOP_DHCP_PACKET,   /* L2_MUX */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_TX_SNOOP_DHCP_PACKET,       /* IML*/
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_BOOTP_PACKET,       /* IML */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_UDP_HELPER_PACKET,  /* IML */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_HSRP_PACKET,        /* IML */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_VRRP_PACKET,        /* IML */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_ARP_PACKET,  /* IML */
#if (SYS_CPNT_DHCPV6SNP == TRUE)
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_DHCPV6SNP_PACKET,  /*IML */
#endif
#if (SYS_CPNT_NDSNP == TRUE)
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_NDSNP_PACKET, /* L2MUX */
#endif
    SYS_CALLBACK_MGR_CALLBACKEVENTID_STACK_STATE,          /* STKTPLG */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_MODULE_STATE_CHANGED, /* STKTPLG */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    SYS_CALLBACK_MGR_CALLBACKEVENTID_UnitHotInsertRemove,
#endif

    SYS_CALLBACK_MGR_CALLBACKEVENTID_SAVING_CONFIG_STATUS, /* STKCTRL */

    SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_CREATED,   /* NETCFG(Triggered by IPCFG) */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_ACTIVE,    /* NETCFG(Triggered by IPCFG) */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_DOWN,      /* NETCFG(Triggered by IPCFG) */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_DESTROYED, /* NETCFG(Triggered by IPCFG) */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_IPCFG_NSM_ROUTE_CHANGE,  /*IPCFG(Triggered by NSM)*/
    SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_L3IF_DESTROY,/*Donny.li modify for VRRP*/

#if (SYS_CPNT_DHCPV6 == TRUE)
    /* NETCFG, Joseph defined for callback DHCPv6 */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_IPV6_SET_MANUAL_ADDR,
    SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_IPV6_ADDRAUTOCONFIG,
    SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_L3IF_CREATE,
    SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_L3IF_OPER_STATUS_UP,
    SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_L3IF_OPER_STATUS_DOWN,
    /* NETCFG */
#endif

    SYS_CALLBACK_MGR_CALLBACKEVENTID_PROVISION_COMPLETE,        /* CLI */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_MODULE_PROVISION_COMPLETE, /* CLI */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_ENTER_TRANSITION_MODE,     /* CLI */

    SYS_CALLBACK_MGR_CALLBACKEVENTID_TELEPHONE_DETECT,      /* LLDP */
#ifdef SYS_CPNT_POE_PSE_DOT3AT
    SYS_CALLBACK_MGR_CALLBACKEVENTID_DOT3AT_INFO_RECEIVED,  /* LLDP */
#endif
#if (SYS_CPNT_DCBX == TRUE)
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_ETS_TLV,       /* LLDP */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_PFC_TLV,       /* LLDP */
#endif /* #if (SYS_CPNT_DCBX == TRUE) */
#if (SYS_CPNT_CN == TRUE)
    SYS_CALLBACK_MGR_CALLBACKEVENTID_CN_REMOTE_CHANGE,      /* LLDP */
#endif

    SYS_CALLBACK_MGR_CALLBACKEVENTID_COS_CHANGED, /* PRIMGMT */

    SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RADIUS_PACKET, /* RADIUS */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RADA_AUTH_RESULT, /* RADIUS */

    SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RADIUS_AUTH_RESULT, /*RADIUS*//*maggie liu for RADIUS authentication ansync*/
    SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RADIUS_AUTH_RESULT_2, /*RADIUS*/

    SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_DOT1X_AUTH_RESULT, /* DOT1X */

    SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_XFER_RESULT, /* Xfer */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_CLI_XFER_RESULT, /* HTTP and SSHD use to callback xfer result */

#if (SYS_CPNT_AMTRL3 == TRUE)
    SYS_CALLBACK_MGR_CALLBACKEVENTID_NEXTHOP_STATUS_CHANGE,  /* AMTRL3 */
#endif

#if (SYS_CPNT_DHCP == TRUE)
    SYS_CALLBACK_MGR_CALLBACKEVENTID_DHCP_RESTART3,          /*DHCP*/
    SYS_CALLBACK_MGR_CALLBACKEVENTID_DHCP_SET_IF_ROLE,        /*DHCP*/
    SYS_CALLBACK_MGR_CALLBACKEVENTID_DHCP_SET_IF_STATUS,      /*DHCP*/
#endif

    SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RELOAD_REMAINING_TIME,    /* SYSMGMT */

#if (SYS_CPNT_CLUSTER == TRUE)
    SYS_CALLBACK_MGR_CALLBACKEVENTID_CLUSTER_CHANGEROLE,      /* CLUSTER */
#endif

#if (SYS_CPNT_CLI_DYNAMIC_PROVISION_VIA_DHCP == TRUE)
    SYS_CALLBACK_MGR_CALLBACKEVENTID_CLI_DYNAMIC_PROVISION_VIA_DHCP,  /*CLI*/
#endif
#if (SYS_CPNT_IP_TUNNEL == TRUE)
        SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_IPV6_PACKET,  /*IPv6 tunneling*/
        SYS_CALLBACK_MGR_CALLBACKEVENTID_TUNNEL_NET_ROUTE_HIT_BIT_CHANGE, /* IPv6 tunneling */
#endif /*SYS_CPNT_IP_TUNNEL*/

#if (SYS_CPNT_POE == TRUE)
    SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_DETECTION_STATUS_CHANGE,     /*POEDRV*/
    SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_STATUS_CHANGE,               /*POEDRV*/
    SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_IS_MAIN_POWER_REACH_MAXIMUM,      /*POEDRV*/
    SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_OVERLOAD_STATUS_CHANGE,      /*POEDRV*/
    SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_POWER_CONSUMPTION_CHANGE,    /*POEDRV*/
    SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_POWER_CLASSIFICATION_CHANGE, /*POEDRV*/
    SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_MAIN_PSE_CONSUMPTION_CHANGE,      /*POEDRV*/
    SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PSE_OPER_STATUS_CHANGE,           /*POEDRV*/
    SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_POWER_DENIED_OCCUR_FRENQUENTLY,   /*POEDRV*/
    SYS_CALLBACK_MGR_CALLBACKEVENTID_POEDRV_PORT_FAILURE_STATUS_CHANGE,       /*POEDRV*/
#endif

    SYS_CALLBACK_MGR_CALLBACKEVENTID_ACL_DELETED,                             /*QoS2*/
    SYS_CALLBACK_MGR_CALLBACKEVENTID_POLICY_MAP_DELETED,                      /*QoS2*/

#if (SYS_CPNT_COS == TRUE)
    SYS_CALLBACK_MGR_CALLBACKEVENTID_COS_PORT_CONFIG_CHANGED,                        /*COS*/
#endif

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
    SYS_CALLBACK_MGR_CALLBACKEVENTID_MGMT_IP_FLT_CHANGED,                     /*mgmt IP filter*/
#endif

    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_RAPS_PACKET,         /* ERPS R-APS  */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_ERPS_HEALTH_PACKET,  /* ERPS Health */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_ERPS_FLUSH_FDB_NOTIFY,       /* ERPS */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_SET_PORT_STATUS,           /* CMGR */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_RAGUARD_PACKET,    /* RA GUARD */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_UDLD_PACKET,       /* UDLD */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_CRAFT_PORT_LINK_UP,      /* SWDRV */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_CRAFT_PORT_LINK_DOWN,    /* SWDRV */

#if (SYS_CPNT_IGMPAUTH == TRUE)
    SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RADIUS_IGMPAUTH_RESULT,                        /*COS*/
#endif
    SYS_CALLBACK_MGR_CALLBACKEVENTID_RX_DHCP_PACKET,/* SYS_CALLBACK_GROUP */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_PFC_CFG_CHANGED,        /* PFC */
    SYS_CALLBACK_MGR_CALLBACKEVENTID_ETS_CFG_CHANGED,        /* ETS */
#if (SYS_CPNT_PBR == TRUE)
    SYS_CALLBACK_MGR_CALLBACKEVENTID_HOST_ROUTE_CHANGED,
    SYS_CALLBACK_MGR_CALLBACKEVENTID_ACL_CHANGED,
    SYS_CALLBACK_MGR_CALLBACKEVENTID_ROUTEMAP_CHANGED,
#endif

};

enum
{
    SYS_CALLBACK_MGR_TRACE_ID_HANDLE_ND_PACKET_CALLBACK,

};

/* for SYS_CALLBACK_MGR_CALLBACKEVENTID_AUTHENTICATE_PACKET
 *
 * Authenticate packet result value.
 *
 * list from low priority to high. higher prirotiy result
 * is able to override lower one.
 *
 * SYS_CALLBACK_MGR_AUTH_BYPASS
 *   initial value.
 *   do nothing and pass this packet to next CSC to authenticate it.
 *
 * SYS_CALLBACK_MGR_AUTH_PENDING
 *   this packet is needed to be processed by other CSC before
 *   determining result of authentication.
 *
 * SYS_CALLBACK_MGR_AUTH_AUTHENTICATED_PERMANENTLY
 *   this packet is authenticated permanently across boots of the system.
 *
 * SYS_CALLBACK_MGR_AUTH_AUTHENTICATED
 *   this packet is authenticated.
 *
 * SYS_CALLBACK_MGR_AUTH_AUTHENTICATED_TEMPORARILY
 *   this packet is authenticated for a limited period only.
 *
 * SYS_CALLBACK_MGR_AUTH_UNAUTHENTICATED
 *   this packet is unauthenticated.
 *
 * SYS_CALLBACK_MGR_AUTH_FAILED
 *   something error, stop authenticating this packet.
 */
typedef enum {
    SYS_CALLBACK_MGR_AUTH_BYPASS,
    SYS_CALLBACK_MGR_AUTH_PENDING,
    SYS_CALLBACK_MGR_AUTH_AUTHENTICATED_PERMANENTLY,
    SYS_CALLBACK_MGR_AUTH_AUTHENTICATED,
    SYS_CALLBACK_MGR_AUTH_AUTHENTICATED_TEMPORARILY,
    SYS_CALLBACK_MGR_AUTH_UNAUTHENTICATED,
    SYS_CALLBACK_MGR_AUTH_FAILED,
} SYS_CALLBACK_MGR_AuthenticatePacket_Result_T;


/* MACRO FUNCTION DECLARATIONS
 */
#define SYS_CALLBACK_MGR_SIZE_OF_MSG(msg_size) (msg_size+sizeof(SYS_CALLBACK_MGR_Msg_T))

/* DATA TYPE DECLARATIONS
 */
/*****************************
 *  callback message header  *
 *****************************
 */
typedef struct SYS_CALLBACK_MGR_Msg_S
{
    UI32_T callback_event_id;
    UI32_T sub_callback_event_id;
    UI32_T cookie;
    UI8_T  callback_data[0];
} SYS_CALLBACK_MGR_Msg_T;

#if (SYS_CPNT_SYS_CALLBACK_SOCKET == TRUE)
 /* Structure that defines the meta data that SYS_CALLBACK_MGR_ReceiveOfPacketCallback includes
  * for sending a received packet to OF Agent. One instance of this structure
  * preceeds the packet itself. */
typedef struct  SYS_CALLBACK_MGR_PktMetaData_s
{
  /* One of OF packet-in reaons
   * if the current 32-bit reason is not enough (actually only 28 bits can be used because of Simba lan use 4 bits for COS)
   * -> add rxReason2 bitmap and add our own bit definition for it
   * use (BCM_RX_REASON_GET(pkt->rx_reasons, bcmRxReasonMcastMiss) == 1) can refer to driverPktReceive() in driver_pkt.c@ofdpa
   *
   */
  UI32_T rx_reason;

  /* One of OFDPA_FLOW_TABLE_ID_t */
  /* uint32_t tableId; no need since we let OF Agent decide the table from the rx_reason
   */

  /* Ingress lport
   */
  UI32_T lport;

  /* Packet length in bytes (not including meta-data)
   */
  UI32_T pkt_length;
} SYS_CALLBACK_MGR_PktMetaData_T;
#endif
/***************************************
 *  callback message type definitions  *
 ***************************************
 */
/* AMTR */
typedef struct
{
    UI32_T src_lport;
    UI16_T vid;
    UI16_T ether_type;
    UI8_T src_mac[6];
    UI8_T dst_mac[6];
} SYS_CALLBACK_MGR_IntrusionMac_CBData_T;

typedef struct
{
    UI32_T number_of_entry;
    UI8_T  buf[0];
} SYS_CALLBACK_MGR_PortMove_CBData_T;

typedef struct
{
    UI32_T ifindex;
    UI32_T vid;
    UI32_T original_ifindex;
    UI8_T mac[6];
} SYS_CALLBACK_MGR_SecurityPortMove_CBData_T;

typedef struct
{
    UI32_T ifindex;
    UI16_T vid;
    UI8_T mac[6];
    BOOL_T is_add;
} SYS_CALLBACK_MGR_MacNotify_CBData_T;

typedef struct
{
    UI32_T ifindex;
    UI32_T portsec_status;
} SYS_CALLBACK_MGR_AutoLearn_CBData_T;

typedef struct
{
    UI32_T ifindex;
    UI32_T reason;
} SYS_CALLBACK_MGR_MACTableDeleteByPort_CBData_T;

typedef struct
{
    UI32_T vid;
} SYS_CALLBACK_MGR_MACTableDeleteByVid_CBData_T;

/* VLAN */
typedef struct
{
    UI32_T vid_ifindex;
    UI32_T vlan_status;
} SYS_CALLBACK_MGR_VlanCreate_CBData_T;

typedef struct
{
    UI32_T vid_ifindex;
    UI32_T vlan_status;
} SYS_CALLBACK_MGR_VlanDestroy_CBData_T;

typedef struct
{
    UI32_T vid_ifindex;
    UI32_T lport_ifindex;
    UI32_T vlan_status;
} SYS_CALLBACK_MGR_VlanMemberAdd_CBData_T;
#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
typedef struct SYS_CALLBACK_MGR_OneUI32Data_S
{
    UI32_T value;
} SYS_CALLBACK_MGR_OneUI32Data_T;

typedef struct SYS_CALLBACK_MGR_OM_TwoUI32Data_S
{
    UI32_T value[2];
} SYS_CALLBACK_MGR_TwoUI32Data_T;

typedef struct SYS_CALLBACK_MGR_ThreeUI32Data_S
{
    UI32_T value[3];
} SYS_CALLBACK_MGR_ThreeUI32Data_T;

typedef struct
{
  union
  {
  UI8_T portlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
  UI8_T vlanlist[((SYS_ADPT_MAX_NBR_OF_VLAN + 7) / 8)];
  }list;
  union
  {
     SYS_CALLBACK_MGR_OneUI32Data_T arg1;
     SYS_CALLBACK_MGR_TwoUI32Data_T arg2;
     SYS_CALLBACK_MGR_ThreeUI32Data_T arg3;
   }arg;

} SYS_CALLBACK_MGR_REFINEList_CBData_T;

#endif

typedef struct
{
    UI32_T vid_ifindex;
    UI32_T lport_ifindex;
    UI32_T vlan_status;
} SYS_CALLBACK_MGR_VlanMemberDelete_CBData_T;

typedef struct
{
    UI32_T lport_ifindex;
    UI32_T old_pvid;
    UI32_T new_pvid;
} SYS_CALLBACK_MGR_PvidChange_CBData_T;

typedef struct
{
    UI32_T vid_ifindex;
    UI32_T oper_status;
} SYS_CALLBACK_MGR_IfOperStatusChanged_CBData_T;

typedef struct
{
    UI32_T dot1q_vlan_index;
    UI32_T trunk_ifindex;
    UI32_T member_ifindex;
} SYS_CALLBACK_MGR_VlanAddFirstTrunkMember_CBData_T;

typedef struct
{
    UI32_T dot1q_vlan_index;
    UI32_T trunk_ifindex;
    UI32_T member_ifindex;
} SYS_CALLBACK_MGR_VlanAddTrunkMember_CBData_T;

typedef struct
{
    UI32_T dot1q_vlan_index;
    UI32_T trunk_ifindex;
    UI32_T member_ifindex;
} SYS_CALLBACK_MGR_VlanDeleteTrunkMember_CBData_T;

typedef struct
{
    UI32_T dot1q_vlan_index;
    UI32_T trunk_ifindex;
    UI32_T member_ifindex;
} SYS_CALLBACK_MGR_VlanDeleteLastTrunkMember_CBData_T;

typedef struct
{
    UI32_T trunk_ifindex;
    UI32_T member_ifindex;
} SYS_CALLBACK_MGR_VlanFinishAddFirstTrunkMember_CBData_T;

typedef struct
{
    UI32_T trunk_ifindex;
    UI32_T member_ifindex;
} SYS_CALLBACK_MGR_VlanFinishAddTrunkMember_CBData_T;

typedef struct
{
    UI32_T trunk_ifindex;
    UI32_T member_ifindex;
} SYS_CALLBACK_MGR_VlanFinishDeleteTrunkMember_CBData_T;

typedef struct
{
    UI32_T trunk_ifindex;
    UI32_T member_ifindex;
} SYS_CALLBACK_MGR_VlanFinishDeleteLastTrunkMember_CBData_T;

typedef struct
{
    UI32_T lport_ifindex;
    UI32_T vlan_port_mode;
} SYS_CALLBACK_MGR_VlanPortMode_CBData_T;

typedef struct
{
    UI32_T vid_ifindex;
    UI32_T lport_ifindex;
    UI32_T vlan_status;
} SYS_CALLBACK_MGR_VlanMemberDeleteByTrunk_CBData_T;

typedef struct
{
    UI32_T vid;
} SYS_CALLBACK_MGR_VlanNameChanged_CBData_T;

typedef struct
{
    UI32_T lport;
} SYS_CALLBACK_MGR_ProtovlanGidBindingChanged_CBData_T;

typedef struct
{
    UI32_T vid_ifindex;
    UI32_T lport_ifindex;
} SYS_CALLBACK_MGR_VlanMemberTagChanged_CBData_T;

/* XSTP */
typedef struct
{
    UI32_T xstid;
    UI32_T lport;
} SYS_CALLBACK_MGR_LportEnterForwarding_CBData_T;

typedef struct
{
    UI32_T xstid;
    UI32_T lport;
} SYS_CALLBACK_MGR_LportLeaveForwarding_CBData_T;
/*add by Tony.Lei for IGMPSnooping*/
typedef struct
{
    BOOL_T is_mstp_mode;
    UI32_T xstid;
    UI32_T lport;
    BOOL_T is_root;
    UI32_T tc_timer;
//    UI8_T  instance_vlans_mapped[512];
} SYS_CALLBACK_MGR_LportTcChange_CBData_T;
typedef struct
{
    UI32_T mode;
    UI32_T status;
} SYS_CALLBACK_MGR_StpChangeVersion_CBData_T;

typedef struct
{
    UI32_T vid;
    UI32_T ifindex;
} SYS_CALLBACK_MGR_MACTableDeleteByVIDnPort_CBData_T;

typedef struct
{
    UI32_T life_time;
} SYS_CALLBACK_MGR_MacTableDeleteByLifeTime_CBData_T;

/* AMTRDRV */
typedef struct
{
    UI32_T number_of_entry;
    UI8_T  addr_buf[0];
} SYS_CALLBACK_MGR_NewMacAddress_CBData_T;

/* NMTRDRV */
typedef struct
{
    UI32_T update_type;
    UI32_T unit;
    UI32_T start_port;
    UI32_T port_amount;
} SYS_CALLBACK_MGR_UpdateLocalNmtrdrvStats_CBData_T;

/* SWCTRL */
typedef struct
{
    UI32_T ifindex;
    UI32_T port_type;
} SYS_CALLBACK_MGR_LPortType_CBData_T;

typedef struct
{
    UI32_T unit;
    UI32_T port;
    UI32_T port_type;
} SYS_CALLBACK_MGR_UPortType_CBData_T;

typedef struct
{
    UI32_T trunk_ifindex;
    UI32_T member_ifindex;
} SYS_CALLBACK_MGR_TrunkMember_CBData_T;

typedef struct
{
    UI32_T ifindex;
} SYS_CALLBACK_MGR_LPort_CBData_T;


typedef struct
{
    UI32_T unit;
    UI32_T port;
} SYS_CALLBACK_MGR_UPort_CBData_T;

typedef struct
{
    UI32_T ifindex;
    UI32_T status_u32;
    BOOL_T status_bool;
} SYS_CALLBACK_MGR_LPortStatus_CBData_T;

typedef struct
{
    UI32_T ifindex;
    UI32_T speed_duplex;
} SYS_CALLBACK_MGR_LPortSpeedDuplex_CBData_T;

typedef struct
{
    UI32_T unit;
    UI32_T port;
    UI32_T speed_duplex;
} SYS_CALLBACK_MGR_UPortSpeedDuplex_CBData_T;

typedef struct
{
    UI32_T unit;
    UI32_T port;
    UI32_T pre_status;
    UI32_T current_status;
} SYS_CALLBACK_MGR_uPortLacpEffectiveOperStatus_CBData_T;

typedef struct
{
    UI32_T ifindex;
    UI32_T pre_status;
    UI32_T current_status;
    UI32_T level;
} SYS_CALLBACK_MGR_PortEffectiveOperStatus_CBData_T;

typedef struct
{
    UI32_T lport;
} SYS_CALLBACK_MGR_IfMauChanged_CBData_T;

typedef struct
{
    UI32_T lport;
    BOOL_T learning;
} SYS_CALLBACK_MGR_PortLearningStatusChanged_CBData_T;

/* SWDRV */
typedef struct
{
    UI32_T unit;
    UI32_T port;
}SYS_CALLBACK_MGR_SWDRV_PortLinkUp_CBData_T;

typedef SYS_CALLBACK_MGR_SWDRV_PortLinkUp_CBData_T SYS_CALLBACK_MGR_SWDRV_PortLinkDown_CBData_T;

typedef struct
{
    UI32_T unit;
}SYS_CALLBACK_MGR_SWDRV_CraftPortLinkUp_CBData_T;

typedef SYS_CALLBACK_MGR_SWDRV_CraftPortLinkUp_CBData_T SYS_CALLBACK_MGR_SWDRV_CraftPortLinkDown_CBData_T;

typedef struct
{
    UI32_T unit;
    UI32_T port;
    UI32_T module_id;
    UI32_T port_type;
}SYS_CALLBACK_MGR_SWDRV_PortTypeChanged_CBData_T;

typedef struct
{
    UI32_T unit;
    UI32_T port;
    UI32_T speed_duplex;
}SYS_CALLBACK_MGR_SWDRV_PortSpeedDuplex_CBData_T;

typedef struct
{
    UI32_T unit;
    UI32_T port;
    UI32_T flow_ctrl;
}SYS_CALLBACK_MGR_SWDRV_PortFlowCtrl_CBData_T;

typedef SYS_CALLBACK_MGR_SWDRV_PortLinkUp_CBData_T SYS_CALLBACK_MGR_SWDRV_HotSwapInsert_CBData_T;

typedef SYS_CALLBACK_MGR_SWDRV_PortLinkUp_CBData_T SYS_CALLBACK_MGR_SWDRV_HotSwapRemove_CBData_T;

typedef struct
{
    UI32_T unit;
    UI32_T port;
    BOOL_T is_present;
}SYS_CALLBACK_MGR_SWDRV_PortSfpPresent_CBData_T;

typedef struct
{
    UI32_T unit;
    UI32_T sfp_index;
    SWDRV_TYPE_SfpInfo_T sfp_info;
}SYS_CALLBACK_MGR_SWDRV_PortSfpInfo_CBData_T;

typedef struct
{
    UI32_T unit;
    UI32_T sfp_index;
    SWDRV_TYPE_SfpDdmInfo_T sfp_ddm_info;
}SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfo_CBData_T;

typedef struct
{
    UI32_T unit;
    UI32_T sfp_index;
    SWDRV_TYPE_SfpDdmInfoMeasured_T sfp_ddm_info_measured;
}SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfoMeasured_CBData_T;

/* IGMPSNP */
typedef struct
{
    UI32_T igmpsnp_status;
} SYS_CALLBACK_MGR_IgmpSnoopStatusChanged_CBData_T;

typedef struct
{
    UI32_T vid_ifidx;
    UI32_T lport_ifidx;
} SYS_CALLBACK_MGR_IgmpSnoopRouterPortChanged_CBData_T;

typedef struct
{
    UI32_T vid_ifidx;
    UI8_T  mip[4];
    UI32_T lport_ifidx;
} SYS_CALLBACK_MGR_IgmpSnoopGroupMemberChanged_CBData_T;

/* SYSMGMT and SYSDRV */
typedef struct
{
    UI32_T unit;
    UI32_T status;
} SYS_CALLBACK_MGR_AlarmInputStatusChanged_CBData_T;

typedef struct
{
    UI32_T unit;
    UI32_T status;
} SYS_CALLBACK_MGR_AlarmOutputStatusChanged_CBData_T;

typedef struct
{
    UI32_T unit;
    UI32_T power;
    UI32_T status;
} SYS_CALLBACK_MGR_PowerStatusChanged_CBData_T;

typedef struct
{
    UI32_T unit;
    UI32_T power;
    UI32_T type;
} SYS_CALLBACK_MGR_PowerTypeChanged_CBData_T;

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
typedef struct
{
    UI32_T unit;
    UI32_T fan;
    UI32_T status;
} SYS_CALLBACK_MGR_FanStatusChanged_CBData_T;
#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
typedef struct
{
    UI32_T unit;
    UI32_T thermal_idx;
    I32_T  temperature;
} SYS_CALLBACK_MGR_ThermalStatusChanged_CBData_T;
#endif

typedef struct
{
    UI32_T unit;
    UI32_T port;
    UI32_T status;
} SYS_CALLBACK_MGR_XFPModuleStatusChanged_CBData_T;

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
typedef struct
{
    UI32_T unit;
    UI32_T fan;
    UI32_T speed;
} SYS_CALLBACK_MGR_FanSpeedChanged_CBData_T;
#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
typedef struct
{
    UI32_T unit;
    UI32_T thermal;
    UI32_T status;
} SYS_CALLBACK_MGR_ThermalTemperatureChanged_CBData_T;
#endif

typedef struct
{
    UI32_T xenpak_type;
} SYS_CALLBACK_MGR_XenpakStatusChanged_CBData_T;

/* TRK */
typedef struct
{
    UI32_T trunk_ifindex;
    UI32_T tm_ifindex;
} SYS_CALLBACK_MGR_AddStaticTrunkMember_CBData_T;

typedef SYS_CALLBACK_MGR_AddStaticTrunkMember_CBData_T SYS_CALLBACK_MGR_DelStaticTrunkMember_CBData_T;

/* LAN */
typedef struct
{
    I32_T   mref_handle_offset;
    UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI16_T  tag_info;
    UI16_T  type;
    UI32_T  pkt_length;
    UI32_T  src_unit;
    UI32_T  src_port;
    UI32_T  packet_class;
} SYS_CALLBACK_MGR_LanReceivePacket_CBData_T;

typedef struct
{
    SYS_CALLBACK_MGR_LanReceivePacket_CBData_T lan_cbdata;
    SYS_CALLBACK_MGR_AuthenticatePacket_Result_T auth_result;
    UI8_T  flag; /* use for sys_callback to select authentication */
} SYS_CALLBACK_MGR_AuthenticatePacket_CBData_T;

/* ISC */
typedef struct
{
    ISC_Key_T isc_key;
    I32_T     mref_handle_offset;
    UI32_T    rx_port;
} SYS_CALLBACK_MGR_IscReceiveStktplgPacket_CBData_T;

/* L2MUX */
typedef struct
{
    I32_T   mref_handle_offset;
    UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI16_T  tag_info;
    UI16_T  type;
    UI32_T  pkt_length;
    UI32_T  lport;
} SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T;


typedef struct
{

    SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T l2mux_cbdata;
    UI32_T auth_count;
    UI32_T auth_csc;
    BOOL_T auth_result;
} SYS_CALLBACK_MGR_RxSnoopDhcpPacket_CBData_T;

/* IML */
typedef struct
{
    I32_T   mref_handle_offset;
    UI32_T  packet_length;
    UI32_T  ifindex;
    UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI16_T  ingress_vid;
    UI32_T  src_port;
} SYS_CALLBACK_MGR_ImlReceivePacket_CBData_T;

typedef struct
{
    UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN];
    I32_T   mref_handle_offset;
    UI32_T  egr_vidifindex;
    UI32_T  egr_lport;
    UI32_T  ing_lport;
    UI32_T  pkt_len;
} SYS_CALLBACK_MGR_TxSnoopDhcpPacket_CBData_T;


/* MLDSNP */
typedef struct
{
    I32_T   mref_handle_offset;
    UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI16_T  tag_info;
    UI16_T  type;
    UI32_T  pkt_length;
    UI32_T  ext_hdr_len; /* ipv6 extension header length */
    UI32_T  lport;
} SYS_CALLBACK_MGR_MldsnpReceivePacket_CBData_T;

/* RA Guard */
typedef struct
{
    I32_T   mref_handle_offset;
    UI32_T  pkt_length;
    UI32_T  src_lport;
    UI16_T  ing_vid;
    UI8_T   ing_cos;
    UI8_T   pkt_type;
    UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN];
} SYS_CALLBACK_MGR_RaGuardReceivePacket_CBData_T;

/* DHCP6SNP */
typedef struct
{
    I32_T   mref_handle_offset;
    UI32_T  packet_length;
    UI32_T  ext_hdr_len;  /* ipv6 extension header length */
    UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI16_T  ingress_vid;
    UI32_T  src_port;
} SYS_CALLBACK_MGR_Dhcp6snpReceivePacket_CBData_T;

/* STKTPLG */
typedef struct SYS_CALLBACK_MGR_StackState_CBData_S
{
    UI32_T msg;
} SYS_CALLBACK_MGR_StackState_CBData_T;

typedef struct SYS_CALLBACK_MGR_ModuleStateChanged_CBData_S
{
    UI32_T unit_id;
} SYS_CALLBACK_MGR_ModuleStateChanged_CBData_T;

/* STKCTRL */
typedef struct SYS_CALLBACK_MGR_SavingConfigStatus_CBData_S
{
    UI32_T status;
} SYS_CALLBACK_MGR_SavingConfigStatus_CBData_T;

/* NETCFG */
/* Triggered by IPCFG
 */
typedef struct SYS_CALLBACK_MGR_RifCreated_CBData_S
{
    UI32_T ifindex;
    L_INET_AddrIp_T addr;
} SYS_CALLBACK_MGR_RifCreated_CBData_T;

typedef struct SYS_CALLBACK_MGR_RifActive_CBData_S
{
    UI32_T ifindex;
    L_INET_AddrIp_T addr;
} SYS_CALLBACK_MGR_RifActive_CBData_T;

typedef struct SYS_CALLBACK_MGR_RifDown_CBData_S
{
    UI32_T ifindex;
    L_INET_AddrIp_T addr;
} SYS_CALLBACK_MGR_RifDown_CBData_T;

typedef struct SYS_CALLBACK_MGR_RifDestroyed_CBData_S
{
    UI32_T ifindex;
    L_INET_AddrIp_T addr;
} SYS_CALLBACK_MGR_RifDestroyed_CBData_T;

#if (SYS_CPNT_DHCPV6 == TRUE)
/* NETCFG, Joseph defined for callback DHCPv6 */
typedef struct SYS_CALLBACK_MGR_IPV6_SetManualAddr_CBData_S
{
    UI32_T  if_index;
    UI32_T  if_mode;
} SYS_CALLBACK_MGR_IPV6_SetManualAddr_CBData_T;

typedef struct SYS_CALLBACK_MGR_IPV6_AddrAutoconfig_CBData_S
{
    UI32_T  if_index;
    UI32_T  if_mode;
} SYS_CALLBACK_MGR_IPV6_AddrAutoconfig_CBData_T;

typedef struct SYS_CALLBACK_MGR_L3IF_Create_CBData_S
{
    UI32_T  if_index;
    UI32_T  if_mode;
} SYS_CALLBACK_MGR_L3IF_Create_CBData_T;

typedef struct SYS_CALLBACK_MGR_L3IF_Destroy_CBData_S
{
    UI32_T  if_index;

} SYS_CALLBACK_MGR_L3IF_Destroy_CBData_T;

typedef struct SYS_CALLBACK_MGR_L3IF_OperStatusUp_CBData_S
{
    UI32_T  if_index;
    UI32_T  if_mode;
} SYS_CALLBACK_MGR_L3IF_OperStatusUp_CBData_T;

typedef struct SYS_CALLBACK_MGR_L3IF_OperStatusDown_CBData_S
{
    UI32_T  if_index;

} SYS_CALLBACK_MGR_L3IF_OperStatusDown_CBData_T;
/* NETCFG */
#endif

typedef struct
{
    UI32_T ifindex;
    BOOL_T bool_v;
} SYS_CALLBACK_MGR_NETCFG_ifindex_bool_v_CBData_T;

typedef struct
{
    UI32_T ifindex;
} SYS_CALLBACK_MGR_NETCFG_ifindex_CBData_T;


/* LLDP */
typedef struct
{
    UI32_T lport;
    UI8_T mac_addr[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T network_addr_subtype;
    UI8_T network_addr[MAXSIZE_lldpLocManAddr];
    UI8_T network_addr_len;
    UI32_T network_addr_ifindex;
    BOOL_T tel_exist;
} SYS_CALLBACK_MGR_TelephoneDetect_CBData_T;

#ifdef SYS_CPNT_POE_PSE_DOT3AT
#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
typedef struct
{
    UI32_T  unit;
    UI32_T  port;
    UI8_T   power_type;
    UI8_T   power_source;
    UI8_T   power_priority;
    UI16_T  power_value;
    UI8_T   requested_power_type;
    UI8_T   requested_power_source;
    UI8_T   requested_power_priority;
    UI16_T  requested_power_value;
    UI8_T   acknowledge;
} SYS_CALLBACK_MGR_ReceiveDot3atInfo_CBData_T;
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
typedef struct
{
    UI32_T  unit;
    UI32_T  port;
    UI8_T   power_type;
    UI8_T   power_source;
    UI8_T   power_priority;
    UI16_T  pd_requested_power;
    UI16_T  pse_allocated_power;
} SYS_CALLBACK_MGR_Dot3atInfoReceived_CBData_T;
#endif
#endif

#if(SYS_CPNT_DCBX == TRUE)
typedef struct
{
    UI32_T  lport;
    BOOL_T  is_delete;
    BOOL_T  rem_recommend_rcvd;
    BOOL_T  rem_willing;
    BOOL_T  rem_cbs;
    UI8_T   rem_max_tc;
    UI8_T   rem_config_pri_assign_table[4];
    UI8_T   rem_config_tc_bandwidth_table[8];
    UI8_T   rem_config_tsa_assign_table[8];
    UI8_T   rem_recommend_pri_assign_table[4];
    UI8_T   rem_recommend_tc_bandwidth_table[8];
    UI8_T   rem_recommend_tsa_assign_table[8];
} SYS_CALLBACK_MGR_DcbxEtsTlvReceived_CBData_T;

typedef struct
{
    UI32_T  lport;
    BOOL_T  is_delete;
    UI8_T   rem_mac[SYS_ADPT_MAC_ADDR_LEN];
    BOOL_T  rem_willing;
    BOOL_T  rem_mbc;
    UI8_T   rem_pfc_cap;
    UI8_T   rem_pfc_enable;
} SYS_CALLBACK_MGR_DcbxPfcTlvReceived_CBData_T;

typedef struct
{
    UI32_T  lport;
} SYS_CALLBACK_MGR_EtsPfcCfgChanged_CBData_T;

#endif

#if (SYS_CPNT_CN == TRUE)
typedef struct
{
    UI32_T  lport;
    UI32_T  neighbor_num;
    UI8_T   cnpv_indicators;
    UI8_T   ready_indicators;
} SYS_CALLBACK_MGR_CnRemoteChange_CBData_T;
#endif /* #if (SYS_CPNT_CN == TRUE) */

/* PRIMGMT */
typedef struct
{
    UI32_T lport_ifindex;
} SYS_CALLBACK_MGR_CosChanged_CBData_T;

/* RADIUS
 *   should be handled by DOT1X_MGR_AnnounceRADIUSPacket
 *
 *   src            ->  SYS_CALLBACK            ->  dst
 *   RADIUS_TASK        NETACCESS_GROUP             DOT1X_MGR
 */

/* copy from radius_client.h
 */
#if ( SYS_CPNT_DOT1X_TTLS_TLS_PEAP == TRUE)
    #define BUFFER_LEN  4096 /* suger, 05-04-2004, max radius packet length is 4096 */
#else
    #define BUFFER_LEN   256
#endif

#if (SYS_CPNT_IGMPAUTH == TRUE)
typedef struct
{
    UI32_T   result;
    UI32_T   auth_port;
    UI32_T   ip_address;
    UI8_T    auth_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T   vlan_id;
    UI32_T   src_ip;
    UI8_T    msg_type;
}SYS_CALLBACK_MGR_AnnounceRadiusIGMPAuthResult_CBData_T;
#endif

typedef struct
{
    UI32_T result;
    UI32_T data_len;
    UI32_T src_port;
    UI32_T session_timeout;
    UI32_T  src_vid;
    UI32_T  server_ip;
    UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T   data_buf[BUFFER_LEN];
    char   authorized_vlan_list[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST+1];
    char   authorized_qos_list [SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE+1];
} SYS_CALLBACK_MGR_AnnounceRADIUSPacket_CBData_T;

/*maggie liu for RADIUS authentication ansync*/
typedef struct
{
    I32_T result;
    I32_T privilege;
    UI32_T cookie;
} SYS_CALLBACK_MGR_AnnounceRADIUSAuthenRadius_CBData_T;

typedef struct
{
    I32_T   result;
    UI32_T  privilege;
    UI8_T   cookie[256];
    UI32_T  cookie_size;
} SYS_CALLBACK_MGR_AnnounceRADIUSAuthenRadius_2_CBData_T;

/* RADA Auth
 *   should be handled by NETACCESS_MGR_AnnounceRadiusAuthorizedResult
 *
 *   src            ->  SYS_CALLBACK            ->  dst
 *   RADIUS_TASK        NETACCESS_GROUP             NETACCESS_MGR
 */
typedef struct
{
    UI32_T  lport;
    UI32_T  session_time;
    UI32_T  server_ip;
    int     identifier;
    UI8_T   authorized_mac[SYS_ADPT_MAC_ADDR_LEN];
    char   authorized_vlan_list[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST+1];
    char   authorized_qos_list [SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE+1];
    BOOL_T  authorized_result;
} SYS_CALLBACK_MGR_AnnounceRadaAuthRes_CBData_T;

/* DOT1X Auth
 *   should be handled by NETACCESS_MGR_AnnounceDot1xAuthorizedResult
 *
 *   src            ->  SYS_CALLBACK            ->  dst
 *   DOT1X_TASK         NETACCESS_GROUP             NETACCESS_MGR
 */
typedef struct
{
    UI32_T  lport;
    UI32_T  session_time;
    UI32_T  server_ip;
    int     eap_identifier;
    UI8_T   authorized_mac[SYS_ADPT_MAC_ADDR_LEN];
    char   authorized_vlan_list[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST+1];
    char   authorized_qos_list [SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE+1];
    UI8_T   authorized_result;
} SYS_CALLBACK_MGR_AnnounceDot1xAuthRes_CBData_T;

/* XFER */
typedef struct
{
    void    *cookie;
    UI32_T  status;
} SYS_CALLBACK_MGR_AnnounceXferResult_CBData_T;

/* HTTP, SSHD */
typedef struct
{
    void    *cookie;
    UI32_T  status;
} SYS_CALLBACK_MGR_AnnounceCliXferResult_CBData_T;

/* AMTRL3 */
#if (SYS_CPNT_AMTRL3 == TRUE)
typedef struct
{
    UI32_T    action_flags;
    UI32_T    fib_id;
    UI32_T    status;
    //IpAddr_T  ip_addr;
    UI32_T    lport_ifindex;
    UI32_T    vid_ifindex;
    UI8_T     dst_mac[SYS_ADPT_MAC_ADDR_LEN];
} SYS_CALLBACK_MGR_Nexthop_Status_Change_CBData_T;
#endif

#if (SYS_CPNT_DHCP == TRUE)
typedef struct
{
    UI32_T restart_object;
} SYS_CALLBACK_MGR_DHCP_Restart3_CBData_T;

typedef struct
{
    UI32_T vid_ifindex;
    UI32_T role;
} SYS_CALLBACK_MGR_DHCP_SetIfRole_CBData_T;

typedef struct
{
    UI32_T vid_ifindex;
    UI32_T status;
} SYS_CALLBACK_MGR_DHCP_SetIfStatud_CBData_T;

#endif

#if (SYS_CPNT_CLI_DYNAMIC_PROVISION_VIA_DHCP == TRUE)
typedef struct
{
    UI32_T  option66_length;
    UI8_T   option66_data[SYS_ADPT_DHCP_MAX_DOMAIN_NAME_LEN];
    UI32_T  option67_length;
    char    option67_data[SYS_ADPT_DHCP_MAX_BOOTFILE_NAME_LEN];
} SYS_CALLBACK_MGR_DHCP_RxOptionConfig_CBData_T;

#endif

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
typedef struct
{
    UI32_T remaining_minutes;
} SYS_CALLBACK_MGR_AnnouceReloadRemainTime_CBData_T;
#endif /* SYS_CPNT_SYSMGMT_DEFERRED_RELOAD */

#if (SYS_CPNT_IP_TUNNEL == TRUE)
typedef struct
{
    UI8_T  source_address[SYS_ADPT_IPV6_ADDR_LEN];
    UI8_T destination_address[SYS_ADPT_IPV6_ADDR_LEN];
} SYS_CALLBACK_MGR_AnnouceIpv6Packet_CBData_T;

typedef struct
{
    UI32_T fib_id;
    UI32_T unit_id;
    UI32_T preflen;
    UI8_T  dst_addr[SYS_ADPT_IPV6_ADDR_LEN];
} SYS_CALLBACK_MGR_TunnelNetRouteHitBitChange_CBData_T;
#endif /* SYS_CPNT_IP_TUNNEL */

#if (SYS_CPNT_POE == TRUE)
typedef struct
{
    UI32_T unit;
    UI32_T port;
    UI32_T value;
} SYS_CALLBACK_MGR_POEDRV_CBData_T;
#endif

#if (SYS_CPNT_CLUSTER == TRUE)
typedef struct
{
    UI32_T role;
} SYS_CALLBACK_MGR_CLUSTER_ChangeRole_CBData_T;
#endif

typedef struct
{
    UI32_T  acl_type;
    char    acl_name[SYS_ADPT_ACL_MAX_NAME_LEN+1];
    UI8_T   dynamic_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
} SYS_CALLBACK_MGR_AnnounceAclDeleted_CBData_T;

typedef struct
{
    char    policy_map_name[SYS_ADPT_DIFFSERV_MAX_NAME_LENGTH+1];
    UI8_T   dynamic_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
} SYS_CALLBACK_MGR_AnnouncePolicyMapDeleted_CBData_T;

#if (SYS_CPNT_COS == TRUE)
typedef struct
{
    UI32_T l_port;
    UI32_T priority_of_config;
} SYS_CALLBACK_MGR_AnnounceCosPortConfigChanged_CBData_T;
#endif

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
typedef struct
{
    UI32_T  mode;
} SYS_CALLBACK_MGR_AnnounceMgmtIPFltChanged_CBData_T;
#endif /* #if (SYS_CPNT_MGMT_IP_FLT == TRUE) */

typedef struct
{
    UI32_T address_family;
} SYS_CALLBACK_MGR_IPCFG_NsmRouteChange_CBData_T;

#if (SYS_CPNT_CFM == TRUE)
typedef struct
{
    UI32_T lport;
    UI32_T mep_id;
    UI16_T vid;
    UI16_T type;
    UI8_T  level;
    BOOL_T defected;
} SYS_CALLBACK_MGR_CFM_DefectNotify_CBData_T;
#endif /* #if (SYS_CPNT_CFM == TRUE) */

typedef struct
{
    UI32_T module_id;
    UI32_T msgq_key;
} SYS_CALLBACK_MGR_Asyn_Return_Msgqkey_List_T;

#if (SYS_CPNT_PBR == TRUE)
typedef struct
{
    L_INET_AddrIp_T addr;
    BOOL_T          is_unresolved;
} SYS_CALLBACK_MGR_HostRouteChanged_CBData_T;
typedef struct
{
    UI32_T acl_index;
    char   acl_name[SYS_ADPT_MAX_ACCESS_LIST_NAME_LENGTH+1];
    UI8_T  type;
#define ACL_CHANGE_TYPE_ADD     1
#define ACL_CHANGE_TYPE_DELETE  2
#define ACL_CHANGE_TYPE_MODIFY  3
} SYS_CALLBACK_MGR_AclChanged_CBData_T;
typedef struct
{
    char   rmap_name[SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1];
    UI32_T seq_num;
    BOOL_T is_deleted;
} SYS_CALLBACK_MGR_RoutemapChanged_CBData_T;
#endif


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
#if (SYS_CPNT_SYS_CALLBACK_SOCKET == TRUE)
void SYS_CALLBACK_MGR_SetSocket(UI32_T module_id, int sockfd);
#endif
/************************
 *  Stacking mode APIs  *
 ************************
 */
/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_CALLBACK_MGR_EnterMasterMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Let SYS_CALLBACK_MGR enter master mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SYS_CALLBACK_MGR_EnterMasterMode(void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_CALLBACK_MGR_EnterSlaveMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Let SYS_CALLBACK_MGR enter slave mode.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SYS_CALLBACK_MGR_EnterSlaveMode(void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_CALLBACK_MGR_EnterTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Let SYS_CALLBACK_MGR enter transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SYS_CALLBACK_MGR_EnterTransitionMode(void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_CALLBACK_MGR_SetTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Set SYS_CALLBACK_MGR to temporary transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *---------------------------------------------------------------------------------*/
void SYS_CALLBACK_MGR_SetTransitionMode(void);

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
 *      src_csc_id       -- The csc_id(SYS_MODULE_AMTR defined in sys_module.h) who triggers this event
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
BOOL_T SYS_CALLBACK_MGR_IntrusionMacCallback(UI32_T src_csc_id, UI32_T src_lport, UI16_T vid, UI8_T *src_mac, UI8_T *dst_mac, UI16_T ether_type);

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
BOOL_T SYS_CALLBACK_MGR_PortMoveCallback(UI32_T src_csc_id, UI32_T num_of_entries, UI8_T *buf, UI32_T buf_size);

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_SecurityPortMoveCallback
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When port move, AMTR will notify other CSC groups by this function.
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_AMTR defined in sys_module.h) who triggers this event
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
BOOL_T SYS_CALLBACK_MGR_SecurityPortMoveCallback(UI32_T src_csc_id, UI32_T ifindex, UI32_T vid, UI8_T  *mac, UI32_T original_ifindex);

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
                                          UI32_T portsec_status);

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
                                                     UI32_T reason);

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
BOOL_T SYS_CALLBACK_MGR_MACTableDeleteByVidCallback(UI32_T src_csc_id, UI32_T vid);

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
                                                         UI32_T ifindex);

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
                                                         UI32_T life_time);


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
                                           UI32_T vlan_status);

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
                                                  UI32_T vlan_status);

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
                                            UI32_T vlan_status);
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_IfOperStatusChangedCallback
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
                                                    UI32_T oper_status);

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
                                            UI32_T vlan_status);

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
                                                   UI32_T vlan_status);

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
                                              UI32_T vlan_status);

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
                                                     UI32_T vlan_status);

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
                                                 UI32_T vlan_status);

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
                                                        UI32_T vlan_status);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_PvidChangeCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when pvid of a port changes.
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
                                           UI32_T new_pvid);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_IfOperStatusChangedCallback
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
BOOL_T SYS_CALLBACK_MGR_IfOperStatusChangedCallback(UI32_T src_csc_id,
                                                    UI32_T vid_ifindex,
                                                    UI32_T oper_status);

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
                                                        UI32_T member_ifindex);

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
                                                   UI32_T member_ifindex);

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
                                                      UI32_T member_ifindex);

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
                                                          UI32_T member_ifindex);

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
                                                              UI32_T member_ifindex);

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
                                                         UI32_T member_ifindex);

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
                                                            UI32_T member_ifindex);

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
                                                                UI32_T member_ifindex);

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
                                             UI32_T vlan_port_mode);

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
                                                        UI32_T vlan_status);

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
BOOL_T SYS_CALLBACK_MGR_VlanNameChangedCallback(UI32_T src_csc_id, UI32_T vid);

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
BOOL_T SYS_CALLBACK_MGR_ProtoVlanGroupIdBindingChangedCallback(UI32_T src_csc_id, UI32_T lport);

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
                                                     UI32_T lport_ifindex);

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
                                                     UI32_T lport);

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
                                                     UI32_T lport);

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
BOOL_T SYS_CALLBACK_MGR_LportChangeStateCallback(UI32_T src_csc_id);
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
BOOL_T SYS_CALLBACK_MGR_LportTcChangeCallback(UI32_T src_csc_id, BOOL_T is_mstp_mode,UI32_T xstid,UI32_T lport,BOOL_T is_root,UI32_T tc_timer,UI8_T *vlan_bit_map);
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
                                                 UI32_T mode, UI32_T status);

/*********************************
 *              SWCTRL           *
 *********************************
 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_PortOperUpCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the oper status is up
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_PortOperUpCallback(
    UI32_T src_csc_id,UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_PortNotOperUpCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the oper status is not up
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_PortNotOperUpCallback(
    UI32_T src_csc_id,UI32_T ifindex);

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
    UI32_T src_csc_id,UI32_T ifindex);

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
    UI32_T src_csc_id,UI32_T ifindex);

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
    UI32_T src_csc_id,UI32_T ifindex,UI32_T speed_duplex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_TrunkMemberAdd1stCallback
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
BOOL_T SYS_CALLBACK_MGR_TrunkMemberAdd1stCallback(
    UI32_T src_csc_id,UI32_T trunk_ifindex,UI32_T member_ifindex);

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
    UI32_T src_csc_id,UI32_T trunk_ifindex,UI32_T member_ifindex);

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
    UI32_T src_csc_id,UI32_T trunk_ifindex,UI32_T member_ifindex);

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
    UI32_T src_csc_id,UI32_T trunk_ifindex,UI32_T member_ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_TrunkMemberPortOperUpCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the oper status is up
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_TrunkMemberPortOperUpCallback(
    UI32_T src_csc_id,UI32_T trunk_ifindex,UI32_T trunk_member_port_ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_TrunkMemberPortNotOperUpCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the oper status is down
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_TrunkMemberPortNotOperUpCallback(
    UI32_T src_csc_id,UI32_T trunk_ifindex,UI32_T trunk_member_port_ifindex);

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
    UI32_T src_csc_id,UI32_T trunk_ifindex,UI32_T trunk_member_port_ifindex);

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
    UI32_T src_csc_id,UI32_T trunk_ifindex,UI32_T trunk_member_port_ifindex);

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
    UI32_T src_csc_id,UI32_T unit,UI32_T port,UI32_T speed_duplex);

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
    UI32_T src_csc_id,UI32_T unit,UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_uPortLinkDownCallback
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
    UI32_T src_csc_id,UI32_T unit,UI32_T port) ;

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
    UI32_T src_csc_id,UI32_T unit,UI32_T port);

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
    UI32_T src_csc_id,UI32_T unit,UI32_T port);

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
    UI32_T src_csc_id, UI32_T ifindex, BOOL_T status, UI32_T changed_bmp);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_PortLinkDownCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the link is down
 * INPUT   : src_csc_id       -- The csc_id who triggers this event
 *           ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SYS_CALLBACK_MGR_PortLinkDownCallback(
    UI32_T src_csc_id,UI32_T ifindex);

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
    UI32_T src_csc_id,UI32_T ifindex);

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
    UI32_T src_csc_id,UI32_T unit,UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_uPortFastLinkDownCallback
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
    UI32_T src_csc_id,UI32_T unit,UI32_T port);

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
    UI32_T src_csc_id,UI32_T ifindex);

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
    UI32_T src_csc_id,UI32_T unit,UI32_T port,UI32_T pre_status, UI32_T current_status );

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
    UI32_T src_csc_id,UI32_T unit, UI32_T port,UI32_T pre_status, UI32_T current_status );

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
    UI32_T level);

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
    UI32_T src_csc_id,UI32_T trunk_ifindex, UI32_T member_ifindex);

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
    UI32_T src_csc_id,UI32_T trunk_ifindex, UI32_T member_ifindex);

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
    UI32_T src_csc_id,UI32_T trunk_ifindex, UI32_T member_ifindex);

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
    UI32_T src_csc_id,UI32_T ifindex, UI32_T port_type);

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
    UI32_T src_csc_id, UI32_T unit, UI32_T port, UI32_T port_type);

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
BOOL_T SYS_CALLBACK_MGR_IfMauChangedCallback(UI32_T src_csc_id, UI32_T lport);

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
BOOL_T SYS_CALLBACK_MGR_PortLearningStatusChangedCallback(UI32_T src_csc_id, UI32_T lport, BOOL_T learning);


/*********************************
 *            AMTRDRV            *
 *********************************
 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_MGR_NewMacAddress
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
BOOL_T SYS_CALLBACK_MGR_AnnounceNewMacAddress(UI32_T src_csc_id, UI32_T num_of_entries, UI8_T *addr_buf, UI32_T buf_size);


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
BOOL_T SYS_CALLBACK_MGR_AgeOutMacAddress(UI32_T src_csc_id, UI32_T num_of_entries, UI8_T *addr_buf, UI32_T buf_size);


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
UI32_T SYS_CALLBACK_MGR_SecurityCheckCallback(UI32_T src_csc_id, UI32_T src_lport, UI16_T vid, UI8_T *src_mac, UI8_T *dst_mac, UI16_T ether_type);

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
BOOL_T SYS_CALLBACK_MGR_UpdateNmtrdrvStats(UI32_T src_csc_id, UI32_T update_type,UI32_T unit, UI32_T start_port, UI32_T port_amount);


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
BOOL_T SYS_CALLBACK_MGR_SWDRV_PortLinkUp(UI32_T src_csc_id, UI32_T unit, UI32_T port);

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
BOOL_T SYS_CALLBACK_MGR_SWDRV_PortLinkDown(UI32_T src_csc_id, UI32_T unit, UI32_T port);

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
BOOL_T SYS_CALLBACK_MGR_SWDRV_CraftPortLinkUp(UI32_T src_csc_id, UI32_T unit);

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
BOOL_T SYS_CALLBACK_MGR_SWDRV_CraftPortLinkDown(UI32_T src_csc_id, UI32_T unit);

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
                                              UI32_T port_type);

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
                                              UI32_T speed_duplex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_MGR_SWDRV_PortFlowCtrl
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
                                           UI32_T flow_ctrl);

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
BOOL_T SYS_CALLBACK_MGR_SWDRV_HotSwapInsert(UI32_T src_csc_id, UI32_T unit, UI32_T port);

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
BOOL_T SYS_CALLBACK_MGR_SWDRV_HotSwapRemove(UI32_T src_csc_id, UI32_T unit, UI32_T port);

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
BOOL_T SYS_CALLBACK_MGR_SWDRV_PortSfpPresent(UI32_T src_csc_id, UI32_T unit, UI32_T port, BOOL_T is_present);

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
BOOL_T SYS_CALLBACK_MGR_SWDRV_PortSfpInfo(UI32_T src_csc_id, UI32_T unit, UI32_T sfp_index, SWDRV_TYPE_SfpInfo_T *sfp_info_p);

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
BOOL_T SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfo(UI32_T src_csc_id, UI32_T unit, UI32_T sfp_index, SWDRV_TYPE_SfpDdmInfo_T *sfp_ddm_info_p);

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
BOOL_T SYS_CALLBACK_MGR_SWDRV_PortSfpDdmInfoMeasured(UI32_T src_csc_id, UI32_T unit, UI32_T sfp_index, SWDRV_TYPE_SfpDdmInfoMeasured_T *sfp_ddm_info_measured_p);

/*********************************
 *             IGMPSNP           *
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
BOOL_T SYS_CALLBACK_MGR_IgmpSnoopStatusChangedCallback(UI32_T src_csc_id, UI32_T igmpsnp_status);

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
BOOL_T SYS_CALLBACK_MGR_IgmpSnoopRouterPortAddCallback(UI32_T src_csc_id, UI32_T vid_ifidx, UI32_T lport_ifidx);

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
BOOL_T SYS_CALLBACK_MGR_IgmpSnoopRouterPortDeleteCallback(UI32_T src_csc_id, UI32_T vid_ifidx, UI32_T lport_ifidx);

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
BOOL_T SYS_CALLBACK_MGR_IgmpSnoopGroupMemberAddCallback(UI32_T src_csc_id, UI32_T vid_ifidx, UI8_T *mip, UI32_T lport_ifidx);

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
BOOL_T SYS_CALLBACK_MGR_IgmpSnoopGroupMemberDeleteCallback(UI32_T src_csc_id, UI32_T vid_ifidx, UI8_T *mip, UI32_T lport_ifidx);

/*********************************
 *             SYSMGMT           *
 *********************************
 */

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_PowerStatusChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When port move, AMTR will notify other CSC groups by this function.
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      unit   --- The unit
 *      power ---
 *      status --- powerstatus
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
BOOL_T SYS_CALLBACK_MGR_PowerStatusChanged_CallBack(UI32_T src_csc_id,UI32_T unit, UI32_T power, UI32_T status);

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_FanStatusChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When port move, AMTR will notify other CSC groups by this function.
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      unit   --- The unit
 *      fan ---
 *      status --- fanstatus
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
BOOL_T SYS_CALLBACK_MGR_FanStatusChanged_CallBack(UI32_T src_csc_id,UI32_T unit, UI32_T fan, UI32_T status);
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
BOOL_T SYS_CALLBACK_MGR_ThermalStatusChanged_CallBack(UI32_T src_csc_id,UI32_T unit, UI32_T thermal_idx, I32_T temperature);
#endif

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_XFPModuleStatusChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When port move, AMTR will notify other CSC groups by this function.
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      unit   --- The unit
 *      port ---
 *      status --- XFPModulestatus
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
BOOL_T SYS_CALLBACK_MGR_XFPModuleStatusChanged_CallBack(UI32_T src_csc_id,UI32_T unit, UI32_T port, UI32_T status);

/*********************************
 *             SYSDRV            *
 *********************************
 */
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
BOOL_T  SYS_CALLBACK_MGR_SYSDRV_AlarmInputStatusChanged_Callback(UI32_T src_csc_id,UI32_T unit, UI32_T status);

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
BOOL_T  SYS_CALLBACK_MGR_SYSDRV_MajorAlarmOutputStatusChanged_Callback(UI32_T src_csc_id,UI32_T unit, UI32_T status);

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
BOOL_T  SYS_CALLBACK_MGR_SYSDRV_MinorAlarmOutputStatusChanged_Callback(UI32_T src_csc_id,UI32_T unit, UI32_T status);

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_SYSDRV_PowerStatusChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When port move, AMTR will notify other CSC groups by this function.
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      unit   --- The unit
 *      power ---
 *      status --- powerstatus
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
BOOL_T  SYS_CALLBACK_MGR_SYSDRV_PowerStatusChanged_Callback(UI32_T src_csc_id,UI32_T unit, UI32_T power, UI32_T status);

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
BOOL_T  SYS_CALLBACK_MGR_SYSDRV_PowerTypeChanged_CallBack(UI32_T src_csc_id,UI32_T unit, UI32_T power, UI32_T type);

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_SYSDRV_FanStatusChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When port move, AMTR will notify other CSC groups by this function.
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      unit   --- The unit
 *      fan ---
 *      status --- fanstatus
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
BOOL_T SYS_CALLBACK_MGR_SYSDRV_FanStatusChanged_CallBack(UI32_T src_csc_id,UI32_T unit, UI32_T fan, UI32_T status);

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_SYSDRV_FanSpeedChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When port move, AMTR will notify other CSC groups by this function.
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      unit   --- The unit
 *      fan ---
 *      speed --- fanspeed
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
BOOL_T SYS_CALLBACK_MGR_SYSDRV_FanSpeedChanged_CallBack(UI32_T src_csc_id,UI32_T unit, UI32_T fan, UI32_T speed);
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
BOOL_T SYS_CALLBACK_MGR_SYSDRV_ThermalStatusChanged_CallBack(UI32_T src_csc_id,UI32_T unit, UI32_T thermal_idx, I32_T temperature);

#endif

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_SYSDRV_XFPModuleStatusChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When port move, AMTR will notify other CSC groups by this function.
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      unit   --- The unit
 *      port ---
 *      status --- XFPModulestatus
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
BOOL_T SYS_CALLBACK_MGR_SYSDRV_XFPModuleStatusChanged_CallBack(UI32_T src_csc_id,UI32_T unit, UI32_T port, UI32_T status);

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_SYSMGMT_XenpakStatusChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When port move, AMTR will notify other CSC groups by this function.
 *
 * INPUT:
 *      src_csc_id       -- The csc_id(SYS_MODULE_XXX defined in sys_module.h) who triggers this event
 *      xenpak_type    ---
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
BOOL_T SYS_CALLBACK_MGR_SYSDRV_XenpakStatusChanged_CallBack(UI32_T src_csc_id,UI32_T xenpak_type);


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
                                                      UI32_T tm_ifindex);

/*-----------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_AddStaticTrunkMember_CallBack
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
                                                      UI32_T tm_ifindex);


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
                                                   UI32_T         packet_class);

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
                                                   UI32_T         packet_class);

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
BOOL_T SYS_CALLBACK_MGR_ReceiveLacpPacketCallback(UI32_T         src_csc_id,
                                                  L_MM_Mref_Handle_T *mref_handle_p,
                                                  UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                  UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                  UI16_T         tag_info,
                                                  UI16_T         type,
                                                  UI32_T         pkt_length,
                                                  UI32_T         src_unit,
                                                  UI32_T         src_port,
                                                  UI32_T         packet_class);


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
BOOL_T SYS_CALLBACK_MGR_ReceiveOamPacketCallback(UI32_T         src_csc_id,
                                                 L_MM_Mref_Handle_T *mref_handle_p,
                                                 UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                 UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                 UI16_T         tag_info,
                                                 UI16_T         type,
                                                 UI32_T         pkt_length,
                                                 UI32_T         src_unit,
                                                 UI32_T         src_port,
                                                 UI32_T         packet_class);

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
                                                 UI32_T         packet_class);

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
                                                  UI32_T         packet_class);

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
    UI32_T         packet_class);

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
    void *cookie);

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
    UI32_T             packet_class);

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
    UI32_T   lport);

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
    UI32_T              ing_lport
);


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
 *           ing_lport       -- ingress lport(0 means sent by DUT)
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
BOOL_T SYS_CALLBACK_MGR_TxSnoopDhcpPacket(
    UI32_T   src_csc_id,
    L_MM_Mref_Handle_T *mref_handle_p,
    UI32_T   pkt_len,
    UI8_T    dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T    src_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI32_T   egr_vidifindex,
    UI32_T   egr_lport,
    UI32_T   ing_lport);



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
BOOL_T SYS_CALLBACK_MGR_ReceiveDot1xPacketCallback(UI32_T         src_csc_id,
                                                  L_MM_Mref_Handle_T *mref_handle_p,
                                                  UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                  UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                  UI16_T         tag_info,
                                                  UI16_T         type,
                                                  UI32_T         pkt_length,
                                                  UI32_T         src_unit,
                                                  UI32_T         src_port,
                                                  UI32_T         packet_class);

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
BOOL_T SYS_CALLBACK_MGR_ReceiveSflowPacketCallback(UI32_T         src_csc_id,
                                                   L_MM_Mref_Handle_T *mref_handle_p,
                                                   UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                   UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                   UI16_T         tag_info,
                                                   UI16_T         type,
                                                   UI32_T         pkt_length,
                                                   UI32_T         src_unit,
                                                   UI32_T         src_port,
                                                   UI32_T         packet_class);

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
                                                 UI32_T             packet_class);

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
                                                     UI32_T         rx_port);


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
                                                 UI32_T         packet_class);

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
#if 0
BOOL_T SYS_CALLBACK_MGR_ReceiveGvrpPacketCallback(UI32_T         src_csc_id,
                                                  L_MM_Mref_Handle_T *mref_handle_p,
                                                  UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                  UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                  UI16_T         tag_info,
                                                  UI16_T         type,
                                                  UI32_T         pkt_length,
                                                  UI32_T         lport);

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
                                                  UI32_T         lport);
#else
BOOL_T SYS_CALLBACK_MGR_ReceiveGvrpPacketCallback(UI32_T         src_csc_id,
                                                 L_MM_Mref_Handle_T *mref_handle_p,
                                                 UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                 UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                 UI16_T         tag_info,
                                                 UI16_T         packet_type,
                                                 UI32_T         pkt_length,
                                                 UI32_T         src_unit,
                                                 UI32_T         src_port,
                                                 UI32_T         packet_class);

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
                                                 UI16_T         packet_type,
                                                 UI32_T         pkt_length,
                                                 UI32_T         src_unit,
                                                 UI32_T         src_port,
                                                 UI32_T         packet_class);

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
                                                 UI32_T         packet_class);
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
                                                     UI32_T         lport);

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
                                                     UI32_T         lport);

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
BOOL_T SYS_CALLBACK_MGR_ReceiveBootpPacketCallback(UI32_T   src_csc_id,
                                                   L_MM_Mref_Handle_T *mref_handle_p,
                                                   UI32_T   packet_length,
                                                   UI32_T   ifindex,
                                                   UI8_T    dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                   UI8_T    src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                   UI16_T   ingress_vid,
                                                   UI32_T   src_port);

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
                                                       UI32_T   src_port);

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
                                                                   UI32_T   src_port);

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
                                                  UI32_T   src_port);

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
                                                  UI32_T   src_port);


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
BOOL_T SYS_CALLBACK_MGR_StackStateCallBack(UI32_T src_csc_id,UI32_T msg);

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
BOOL_T SYS_CALLBACK_MGR_ModuleStateChangedCallBack(UI32_T src_csc_id,UI32_T unit_id);

#if (SYS_CPNT_CFM == TRUE)
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
                                                     UI32_T         lport);

#endif /* #if (SYS_CPNT_CFM == TRUE) */

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
BOOL_T SYS_CALLBACK_MGR_UnitHotInsertRemoveCallBack (UI32_T src_csc_id,UI32_T msg);

#endif

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
BOOL_T SYS_CALLBACK_MGR_SavingConfigStatusCallBack(UI32_T src_csc_id, UI32_T status);

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
    UI32_T              packet_class);

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
BOOL_T SYS_CALLBACK_MGR_RifCreatedCallBack(UI32_T src_csc_id, UI32_T ifindex, L_INET_AddrIp_T *addr_p);

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_RifActiveCallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When RIF is activated, IPCFG will notify this event.
 *
 * INPUT:
 *      src_csc_id -- The csc_id(SYS_MODULE_STKTPLG defined in sys_module.h) who
 *                    triggers this event
 *      ifindex      -- ifindex
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
BOOL_T SYS_CALLBACK_MGR_RifActiveCallBack(UI32_T src_csc_id, UI32_T ifindex, L_INET_AddrIp_T *addr_p);

/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_RifDownCallBack
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      When RIF is down, IPCFG will notify this event.
 *
 * INPUT:
 *      src_csc_id -- The csc_id(SYS_MODULE_STKTPLG defined in sys_module.h) who
 *                    triggers this event
 *      ifindex      -- ifindex
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
BOOL_T SYS_CALLBACK_MGR_RifDownCallBack(UI32_T src_csc_id, UI32_T ifindex, L_INET_AddrIp_T *addr_p);

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
BOOL_T SYS_CALLBACK_MGR_RifDestroyedCallBack(UI32_T src_csc_id, UI32_T ifindex, L_INET_AddrIp_T *addr_p);

#if (SYS_CPNT_DHCPV6 == TRUE)
/*-------------------------------------------------------------------------
 *FUNCTION NAME : SYS_CALLBACK_MGR_NETCFG_IPv6AddrAutoConfig
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      IPv6 address autoconfig enable/disable notification from NETCFG.
 *
 * INPUT:
 *      ifindex     -- the L3 interface which IPv6 address is configured manually.
 *      status      -- the status of IPv6AddrAutoconfig.
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
BOOL_T SYS_CALLBACK_MGR_NETCFG_IPv6AddrAutoConfig(UI32_T src_csc_id, UI32_T ifindex, BOOL_T status);

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
BOOL_T SYS_CALLBACK_MGR_NETCFG_L3IfCreate(UI32_T src_csc_id, UI32_T ifindex, BOOL_T status);


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
BOOL_T SYS_CALLBACK_MGR_NETCFG_L3IfOperStatusUp(UI32_T src_csc_id, UI32_T ifindex, BOOL_T status);

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
BOOL_T SYS_CALLBACK_MGR_NETCFG_L3IfOperStatusDown(UI32_T src_csc_id, UI32_T ifindex);

#endif

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
BOOL_T SYS_CALLBACK_MGR_NETCFG_L3IfDestroy(UI32_T src_csc_id, UI32_T ifindex);
/*Donny.li modify for VRRP end*/



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
BOOL_T SYS_CALLBACK_MGR_ProvisionCompleteCallback(UI32_T src_csc_id);

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
BOOL_T SYS_CALLBACK_MGR_ModuleProvisionCompleteCallback(UI32_T src_csc_id,UI32_T unit);

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
BOOL_T SYS_CALLBACK_MGR_EnterTransitionModeCallback(UI32_T src_csc_id);

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
                                                BOOL_T tel_exist);

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
                                                  UI8_T   acknowledge);
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
                                                  UI16_T   pse_allocated_power);
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
        UI32_T neighbor_num, UI8_T cnpv_indicators, UI8_T ready_indicators);
#endif /* #if (SYS_CPNT_CN == TRUE) */

/*********************************
 *            PRIMGMT            *
 *********************************
 */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_CosChangedCallback
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
BOOL_T SYS_CALLBACK_MGR_CosChangedCallback(UI32_T src_csc_id, UI32_T lport_ifindex);

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
BOOL_T SYS_CALLBACK_MGR_AnnounceXferResultCallback(UI32_T src_csc_id, UI32_T dest_msgq, void *fun, void *arg_cookie, UI32_T arg_status);

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

BOOL_T SYS_CALLBACK_MGR_AnnounceCliXferResultCallback(UI32_T src_csc_id, UI32_T dest_msgq, void *fun, void *arg_cookie, UI32_T arg_status);

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
        UI8_T       port_list[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST]);

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
        UI8_T       dynamic_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

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
        UI32_T priority_of_config);
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
//BOOL_T SYS_CALLBACK_MGR_NextHopStatusChangeCallback(UI32_T src_csc_id, UI32_T action_flags, UI32_T fib_id, UI32_T status, IpAddr_T ip_addr, UI32_T lport_ifindex, UI32_T vid_ifindex, UI8_T *dst_mac);
#endif

/*********************************
 *            RADIUS             *
 *********************************
 */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_AnnounceRADIUSPacket
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when cos mapping changed.
 *
 * INPUT   : src_csc_id    -- The csc_id who triggers this event ()
 *
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
    char    *authorized_vlan_list,  char    *authorized_qos_list,
    UI32_T  session_timeout,        UI32_T  server_ip);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_AnnounceRadiusAuthorizedResult
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when radius authoirized result is received.
 * INPUT   :
 * OUTPUT  : None.
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_MGR_AnnounceRadiusAuthorizedResult(
    UI32_T  src_csc_id,             UI32_T  dest_msgq_key,
    UI32_T  lport,                  UI8_T   *mac,
    int     identifier,             BOOL_T  authorized_result,
    UI8_T   *authorized_vlan_list,  UI8_T   *authorized_qos_list,
    UI32_T  session_time,           UI32_T  server_ip);

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
    UI8_T    msg_type
);
#endif

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
    UI32_T  session_time,   UI32_T  server_ip);


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
    UI32_T              lport);
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
BOOL_T SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(SYSFUN_Msg_T* ipcmsg_p, void* callback_fun);


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
BOOL_T SYS_CALLBACK_MGR_GetAndClearFailInfo(UI32_T cscgroup_mgr_msgqkey, SYS_CALLBACK_OM_FailEntry_T *fail_entry_p);
#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
#define SYS_CALLBACK_MGR_IS_MEMBER(list, member)  (((list[((member) - 1) >> 3]) & (1 << (7 - (((member) - 1) & 7)))) != 0)

BOOL_T SYS_CALLBACK_MGR_ProcessRefineOmDB();

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
BOOL_T SYS_CALLBACK_MGR_VlanMemberListAddCallback(UI32_T src_csc_id,
                                              UI8_T *vidlist,
                                              UI32_T lport_ifindex,
                                              UI32_T vlan_status);


#endif

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_DHCPRestart3Callback
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when restart dhcp
 *
 *
 * INPUT   : src_csc_id    -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           restart_object   -- restart client \relay \ server
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : this api only used by  underlayer  module such as netcfg send msg to dhcp module
 *-----------------------------------------------------------------------------
 */

BOOL_T SYS_CALLBACK_MGR_DHCPRestart3Callback(UI32_T src_csc_id,
                                                                  UI32_T restart_object);
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_MGR_DHCPSetIfRoleCallback
 *-----------------------------------------------------------------------------
 * PURPOSE : Call CallBack function when set interface role
 *           set.
 *
 * INPUT   : src_csc_id    -- The csc_id who triggers this event (SYS_MODULE_VLAN)
 *           vid_ifindex   -- specify which vlan member set to join
 *           role --     interface role (client , relay)
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Callback messages have been sent successfully
 *           FALSE -- Fail to send some of the callback messages
 *
 * NOTES   : this api only used by  underlayer  module such as netcfg send msg to dhcp module
 *-----------------------------------------------------------------------------
 */

BOOL_T SYS_CALLBACK_MGR_DHCPSetIfRoleCallback(UI32_T src_csc_id,
                                                                  UI32_T vid_ifindex,
                                                                  UI32_T role);

BOOL_T SYS_CALLBACK_MGR_DHCPSetIfStatusCallback(UI32_T src_csc_id,
                                                                  UI32_T vid_ifindex,
                                                                  UI32_T status);
#if (SYS_CPNT_CLI_DYNAMIC_PROVISION_VIA_DHCP== TRUE)
BOOL_T SYS_CALLBACK_MGR_DHCP_RxOptionConfigCallback(UI32_T src_csc_id,
                                                                  UI32_T option66_length,
                                                                  UI8_T  *option66_data,
                                                                  UI32_T option67_length,
                                                                  char   *option67_data);
#endif

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
BOOL_T SYS_CALLBACK_MGR_AnnounceReloadReaminTime(
    UI32_T src_csc_id,
    UI32_T remain_minutes);
#endif

/*maggie liu for RADIUS authentication ansync*/
BOOL_T SYS_CALLBACK_MGR_AnnounceRADIUSAuthenResult(UI32_T  src_csc_id,
    UI32_T  cookie, I32_T  result, I32_T privilege);

BOOL_T SYS_CALLBACK_MGR_AnnounceRemServerAuthResult(
    UI32_T  src_csc_id,
    I32_T result,
    UI32_T privilege,
    void *cookie,
    UI32_T cookie_size
);

#if (SYS_CPNT_IP_TUNNEL == TRUE)
BOOL_T SYS_CALLBACK_MGR_AnnounceIpv6PacketCallback(UI32_T  src_csc_id, UI8_T* src_addr,UI8_T* dst_addr);
BOOL_T SYS_CALLBACK_MGR_TunnelNetRouteHitBitChangeCallback(UI32_T  src_csc_id,UI32_T fib_id, UI8_T *dst_addr, UI32_T preflen, UI32_T unit_id);
#endif /*SYS_CPNT_IP_TUNNEL*/

#if (SYS_CPNT_CLUSTER == TRUE)
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
                                                     UI32_T         lport);

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
BOOL_T SYS_CALLBACK_MGR_ChangeClusterRoleCallback(UI32_T   src_csc_id,UI32_T   role );

#endif /* SYS_CPNT_CLUSTER */

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
BOOL_T SYS_CALLBACK_MGR_POEDRV_PortDetectionStatusChange(UI32_T src_csc_id, UI32_T unit, UI32_T port, UI32_T status);

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
BOOL_T SYS_CALLBACK_MGR_POEDRV_PortStatusChange(UI32_T src_csc_id, UI32_T unit, UI32_T port, UI32_T status);

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
BOOL_T SYS_CALLBACK_MGR_POEDRV_IsMainPowerReachMaximun(UI32_T src_csc_id, UI32_T unit, UI32_T status);

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
BOOL_T SYS_CALLBACK_MGR_POEDRV_PortOverloadStatusChange(UI32_T src_csc_id, UI32_T unit, UI32_T port, UI32_T status);

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
BOOL_T SYS_CALLBACK_MGR_POEDRV_PortPowerConsumptionChange(UI32_T src_csc_id, UI32_T unit, UI32_T port, UI32_T power);

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
BOOL_T SYS_CALLBACK_MGR_POEDRV_PortPowerClassificationChange(UI32_T src_csc_id, UI32_T unit, UI32_T port, UI32_T classification);

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
BOOL_T SYS_CALLBACK_MGR_POEDRV_MainPseConsumptionChange(UI32_T src_csc_id, UI32_T unit, UI32_T power);

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
BOOL_T SYS_CALLBACK_MGR_POEDRV_PseOperStatusChange(UI32_T src_csc_id, UI32_T unit, UI32_T status);

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
BOOL_T SYS_CALLBACK_MGR_POEDRV_PowerDeniedOccurFrequently(UI32_T src_csc_id, UI32_T unit, UI32_T port);

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
BOOL_T SYS_CALLBACK_MGR_POEDRV_PortFailureStatusChange(UI32_T src_csc_id, UI32_T unit, UI32_T port, UI32_T status);
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
BOOL_T SYS_CALLBACK_MGR_AnnounceMgmtIPFltChanged(UI32_T src_csc_id, UI32_T mode);
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
BOOL_T SYS_CALLBACK_MGR_IPCFG_NsmRouteChange(UI32_T src_csc_id, UI32_T address_family);

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
    UI32_T src_csc_id, UI32_T ifindex, BOOL_T status, UI32_T reason);

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
                                        BOOL_T defected);
#endif

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
 *      eth_type       --- ether type
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
                                                     UI32_T   src_lport);
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE) */

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
                                                   UI32_T   src_port);
#endif /* SYS_CPNT_DHCPV6SNP */


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
                                                UI32_T   port_no);

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
                                           UI16_T   egress_vid);

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
void SYS_CALLBACK_MGR_BackDoorMenu(void);

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
                                                    UI8_T   *rem_recommend_tsa_assign_table);

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
                                                    UI8_T  rem_pfc_enable);

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
    UI32_T  lport);

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
    UI32_T  lport);

#endif /* End of #if(SYS_CPNT_DCBX == TRUE) */

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
BOOL_T SYS_CALLBACK_MGR_MlagMacUpdateCallback(UI32_T src_csc_id, UI32_T  dest_msgq_key, UI32_T ifindex, UI16_T vid, UI8_T *mac_p, BOOL_T is_add);

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
                                                  UI32_T             packet_class);
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
BOOL_T SYS_CALLBACK_MGR_MacAddrUpdateCallback(UI32_T src_csc_id, UI32_T ifindex, UI16_T vid, UI8_T *mac_p, BOOL_T is_add);

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
BOOL_T SYS_CALLBACK_MGR_HostRouteChanged(UI32_T src_csc_id, L_INET_AddrIp_T *addr_p, BOOL_T is_unresolved);

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
BOOL_T SYS_CALLBACK_MGR_AclChanged(UI32_T src_csc_id, UI32_T acl_index, char *acl_name_p, UI8_T type);

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
BOOL_T SYS_CALLBACK_MGR_RouteMapChanged(UI32_T src_csc_id, char *rmap_name, UI32_T seq_num, BOOL_T is_deleted);
#endif

#endif    /* End of SYS_CALLBACK_MGR_H */
