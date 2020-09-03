/* =====================================================================================*
 * FILE NAME: VLAN_TYPE.h                                                               *
 *                                                                                      *
 * ABSTRACT:  This file contains the defined datatypes for vlan.
 *                                                                                      *
 * MODIFICATION HISOTRY:                                                                *
 *                                                                                      *
 * MODIFIER        DATE        DESCRIPTION                                              *
 * -------------------------------------------------------------------------------------*
 * amytu        12-25-2001     First Create                                             *
 *                                                                                      *
 * -------------------------------------------------------------------------------------*
 * Copyright(C)        Accton Techonology Corporation 2001                              *
 * =====================================================================================*/

#ifndef VLAN_TYPE_H
#define VLAN_TYPE_H


#include "sys_type.h"
#include "leaf_2674q.h"
#include "leaf_2863.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"


/* NAMING CONSTANT DECLARATIONS
 */

#define VLAN_TYPE_1V_UI32_ULTRA_VALUE           0xFFFFFFFF
#define VLAN_TYPE_DOT1V_PROTOCOL_VALUE_IPV4     0x0800
#define VLAN_TYPE_DOT1V_PROTOCOL_VALUE_ARP      0x0806
#define VLAN_TYPE_DOT1V_PROTOCOL_VALUE_RARP     0x8035
#define VLAN_TYPE_DOT1V_PROTOCOL_VALUE_IPV6     0X86DD
#define VLAN_TYPE_DOT1V_PROTOCOL_VALUE_IPXRAW   0xFFFF
#define VLAN_TYPE_DOT1V_PROTOCOL_VALUE_PPPOE_DISCOVER    0x8863
#define VLAN_TYPE_DOT1V_PROTOCOL_VALUE_PPPOE_SESSION     0x8864

#define VLAN_TYPE_DOT1Q_NULL_VLAN_ID            0x0000
#define VLAN_TYPE_DOT1Q_DEFAULT_PVID            0x0001
#define VLAN_TYPE_DOT1Q_RESERVED_VLAN_ID        0x0FFF

#define VLAN_TYPE_EVENT_ENTER_TRANSITION        0x0001L
#define VLAN_TYPE_PORTSTATE_EVENT               0x0002L   /* callback msg event        */

#define VLAN_OM_1V_MAX_1V_PROTOCOL_VALUE_LENGTH         SYS_ADPT_1V_MAX_PROTOCOL_VALUE_BUFFER_LENGTH

#define VLAN_TYPE_ALIAS_NAME_LEN    64

#define VLAN_TYPE_VLAN_LIST_SIZE    (SYS_DFLT_DOT1QMAXVLANID + 7) / 8

enum
{
    VLAN_TYPE_VLAN_STATUS_NONE      = 0,
    VLAN_TYPE_VLAN_STATUS_OTHER     = VAL_dot1qVlanStatus_other,
    VLAN_TYPE_VLAN_STATUS_STATIC    = VAL_dot1qVlanStatus_permanent,
    VLAN_TYPE_VLAN_STATUS_GVRP      = VAL_dot1qVlanStatus_dynamicGvrp,
    VLAN_TYPE_VLAN_STATUS_AUTO      = 4,
    VLAN_TYPE_VLAN_STATUS_VOICE     = 5,
    VLAN_TYPE_VLAN_STATUS_MVR       = 6,
    VLAN_TYPE_VLAN_STATUS_MVR6      = 7
};

/* for trace_id of user_id when allocate buffer with l_mm
 */
enum
{
    VLAN_TYPE_TRACE_ID_VLAN_MGR_ADDVLANLIST = 0,
    VLAN_TYPE_TRACE_ID_VLAN_MGR_SET_IP_SUNBNET_VLAN_ENTRY,
    VLAN_TYPE_TRACE_ID_GET_RUNNING_PROM_PORT_PARAMETERS
};

enum
{
    VLAN_MGR_IP_STATE_NONE = 0,
    VLAN_MGR_IP_STATE_IPV4,
    VLAN_MGR_IP_STATE_IPV6,
    VLAN_MGR_IP_STATE_UNCHANGED     /* used for option when setting, never saved in OM */
};

enum
{
    VLAN_MGR_RETURN_ZERO = 0
};

/* return value definition
 */
enum
{
    VLAN_TYPE_RETVAL_OK,
    VLAN_TYPE_RETVAL_INVALID_ARG,
    VLAN_TYPE_RETVAL_CREATE_VLAN_DEV_FAIL,
    VLAN_TYPE_RETVAL_VLAN_DEV_NOT_EXISTS,
    VLAN_TYPE_RETVAL_UNKNOWN_SYSCALL_CMD,
    VLAN_TYPE_RETVAL_UNKNOWN_ERROR
};

/* vlan system call command definition
 */
enum
{
    VLAN_TYPE_SYSCALL_CMD_CREATE_VLAN_DEV,
    VLAN_TYPE_SYSCALL_CMD_DESTROY_VLAN_DEV
};

/* DATA TYPE DECLARATIONS
 */

typedef enum /* function number */
{
    VLAN_TYPE_VLAN_TASK_TASK_FUNCTION_NUMBER = 0,
    VLAN_TYPE_VLAN_TASK_DISPATCH_MSG_FUNCTION_NUMBER,
    VLAN_TYPE_VLAN_MGR_ROWSTATUS_FUNCTION_NUMBER,
    VLAN_TYPE_VLAN_OM_FUNCTION_NUMBER
} VLAN_TYPE_Vlan_Function_Number_T ;

typedef enum /* error code */
{
    VLAN_TYPE_VLAN_TASK_ERROR_NUMBER = 0,
    VLAN_TYPE_VLAN_TASK_DISPATCH_MSG_ERROR_NUMBER,
    VLAN_TYPE_VLAN_MGR_ROWSTATUS_ERROR_NUMBER,
    VLAN_TYPE_VLAN_OM_FUNCTION_ERROR_NUMBER
} VLAN_TYPE_Vlan_Error_Number_T;

#if (SYS_CPNT_MAC_VLAN == TRUE)

/* The Entry of MAC-to-VLAN mapping table */
typedef struct
{
    UI16_T      vid;
    UI8_T       mac_address[SYS_ADPT_MAC_ADDR_LEN];    /*key*/
    UI8_T       mask[SYS_ADPT_MAC_ADDR_LEN];           /*key*/
    UI8_T       priority;

    UI8_T       status;     /*only used by MIB */
}VLAN_TYPE_MacVlanEntry_T;

#endif /* end of #if (SYS_CPNT_MAC_VLAN == TRUE)*/

typedef enum
{
    VLAN_MGR_UNTAGGED_ONLY,
    VLAN_MGR_TAGGED_ONLY,
    VLAN_MGR_BOTH,
    VLAN_MGR_PERMANENT_ONLY,
    VLAN_MGR_DYNAMIC_ONLY,
} VLAN_TYPE_MemberType_T;

/*=============================================================================
 * Moved from vlan_mgr.h
 *=============================================================================
 */

typedef struct
{
    UI32_T  dot1q_vlan_index;       /*Primary search key for Dot1qVlanstatictable. */

    char    dot1q_vlan_static_name[SYS_ADPT_MAX_VLAN_NAME_LEN+1];
    UI8_T   dot1q_vlan_static_egress_ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T   dot1q_vlan_static_untagged_ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T   dot1q_vlan_forbidden_egress_ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI32_T  dot1q_vlan_static_row_status;
} VLAN_MGR_Dot1qVlanStaticEntry_T;

typedef struct
{
    UI32_T  vid_ifindex;            /* KEY */

    /* NON-DEFAULT VALUE */
    char    vlan_name[SYS_ADPT_MAX_VLAN_NAME_LEN+1];
    char    vlan_alias[VLAN_TYPE_ALIAS_NAME_LEN+1];
    UI8_T   vlan_egress_ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T   vlan_untagged_ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T   vlan_forbidden_egress_ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI32_T  vlan_row_status;
    UI32_T  vlan_address_method;

#if (SYS_CPNT_RSPAN == TRUE)
    UI8_T   rspan_status;
#endif

    /* Changed Status */
    BOOL_T  name_changed;
    BOOL_T  alias_changed;
    BOOL_T  egress_ports_changed;
    BOOL_T  untag_ports_changed;
    BOOL_T  forbidden_ports_changed;
    BOOL_T  row_status_changed;
    BOOL_T  address_method_changed;

#if (SYS_CPNT_RSPAN == TRUE)
    BOOL_T  rspan_status_changed;
#endif

} VLAN_TYPE_Vlan_RunningCfg_T;

typedef struct
{
    UI32_T  lport_ifindex;          /* KEY */

    /* NON-DEFAULT VALUE */
    UI32_T  pvid_index;
    UI32_T  port_acceptable_frame_types;
    UI32_T  port_ingress_filtering;
    UI32_T  port_gvrp_status;
    UI32_T  vlan_port_mode;         /* Q-Trunk */
    BOOL_T  port_trunk_mode;        /* Port Trunk indication */

#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
    BOOL_T  port_trunk_link_mode;
#endif

    /* Changed Status */
    BOOL_T  pvid_changed;
    BOOL_T  acceptable_frame_types_changed;
    BOOL_T  ingress_filtering_changed;
    BOOL_T  port_gvrp_status_changed;
    BOOL_T  vlan_port_mode_changed;
    BOOL_T  port_trunk_mode_changed;

#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
    BOOL_T  port_trunk_link_mode_changed;
#endif

} VLAN_TYPE_Vlan_Port_RunningCfg_T;


/*=============================================================================
 * Moved from vlan_om.h
 *=============================================================================
 */

/* Specify a port is which kind of member of a VLAN
 */
typedef enum
{
    VLAN_OM_VlanMemberType_CurrentUntagged  = 0,
    VLAN_OM_VlanMemberType_CurrentEgress    = 1,
    VLAN_OM_VlanMemberType_StaticEgress     = 2,
    VLAN_OM_VlanMemberType_ForbiddenEgress  = 3
} VLAN_OM_VlanMemberType_T;

/* RFC 2863 Definition
 *Every interface has an attribute - ifType that determine the type of interface and is defined in RFC2863.
 *For VLAN interface, there are two reasonable values: l2vlan (135) and l3ipvlan (136).
 *l2vlan (135) refers to Layer 2 Virtual LAN using 802.1Q.
 *The VLAN whose ifType is l2vlan (135) has not any L3 related configuration and
 *can't make any L3 related configuration except after changed to l3ipvlan (136).
 *l3ipvlan (136) refers to Layer 3 Virtual LAN using IP.
 *The VLAN whose ifType is l3ipvlan (136) can make L3 related configuration
 *and normally is bound to one fixed IP address or get IP address from DHCP or BOOTP server.
 * ADD by Tony.Lei
 */
#define VLAN_L2_IFTYPE  1
#define VLAN_L3_IP_IFTYPE 2
#define VLAN_L3_IP_TUNNELTYPE  3
#define VLAN_DEFAULT_IFTYPE  VLAN_L2_IFTYPE
typedef struct
{
    UI32_T  ifType ;
    UI8_T   vlan_operation_status;
    UI32_T  admin_status;          /* read-only */
    UI32_T  link_up_down_trap_enabled;
    UI32_T  if_last_change;

} VLAN_OM_IfEntry_T;

typedef struct
{
    /* Dot1qVlanCurrentEntry defined in RFC 2674 page 59~61
     */
    /* The value of sysUpTime when this VLAN was created. */
    UI32_T  dot1q_vlan_creation_time;
    /* The value of time when this VLAN was modified. */
    UI32_T  dot1q_vlan_time_mark;
    /* The VLAN ID */
    UI16_T  dot1q_vlan_index;
    /* The Filtering Database used by this VLAN. */
    UI32_T  dot1q_vlan_fdb_id;
    /* The set of ports which are permanently or dynamically assigned to this VLAN as tagged or untagged member.
	  (The set of ports which are transmitting traffic for this VLAN as either tagged or untagged frames.) */
    UI8_T   dot1q_vlan_current_egress_ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    /* The set of ports which are permanently or dynamically assigned to this VLAN as untagged member.
	  (The set of ports which are transmitting traffic for this VLAN as untagged frames.) */
    UI8_T   dot1q_vlan_current_untagged_ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    /* Indicate the status of vlan:
        VAL_dot1qVlanStatus_other
        VAL_dot1qVlanStatus_permanent
        VAL_dot1qVlanStatus_dynamicGvrp
    */
    UI8_T   dot1q_vlan_status;

    /* Dot1qVlanStaticEntry defined in RFC2764 page 62~63.  Subset of Dot1qVlanCurrentEntry.
     */
    /* The set of ports which are permanently assigned to this VLAN as tagged or untagged.(It dose not mean
       this VLAN is static created.)Thus static_egress_ports is subset of current_egress_ports. */
    UI8_T   dot1q_vlan_static_egress_ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    /* The set of ports which are permanently assigned to this VLAN as untagged.
       Thus static_untagged_ports is subset of current_untagged_ports. */
    UI8_T   dot1q_vlan_static_untagged_ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    /* The set of ports which are prohibited by management from being included in the egress list for this VLAN. */
    UI8_T   dot1q_vlan_forbidden_egress_ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    /* This object indicates the rowstatus of this entry.
        VAL_dot1qVlanStaticRowStatus_active
        VAL_dot1qVlanStaticRowStatus_notInService
        VAL_dot1qVlanStaticRowStatus_notReady
        VAL_dot1qVlanStaticRowStatus_createAndGo
        VAL_dot1qVlanStaticRowStatus_createAndWait
        VAL_dot1qVlanStaticRowStatus_destroy
    */
    UI32_T  dot1q_vlan_static_row_status;
    /* An administratively assigned string, which may be used to identify the VLAN. */
    char    dot1q_vlan_static_name[SYS_ADPT_MAX_VLAN_NAME_LEN+1];
    char    dot1q_vlan_alias[MAXSIZE_ifAlias+1];
    /* Es3626a  Private Mib requirement
     */
    /* The way to set vlan address:
        VAL_vlanAddressMethod_user(Default)
        VAL_vlanAddressMethod_bootp
        VAL_vlanAddressMethod_dhcp
    */
    UI32_T  vlan_address_method;

    /* specify whether is ip interface or not and which version
        VLAN_MGR_IP_STATE_NONE,(Default)
        VLAN_MGR_IP_STATE_IPV4,
        VLAN_MGR_IP_STATE_IPV6,
        VLAN_MGR_IP_STATE_UNCHANGED
    */
    UI8_T   vlan_ip_state;

    /* RFC 2863 Definition
     */
    VLAN_OM_IfEntry_T   if_entry;

  /*on the LINUX platform, add the feature of logical mac  to L3 vlan,
     *Tony.Lei
     */
    UI8_T   cpu_mac[SYS_ADPT_MAC_ADDR_LEN];

#if (SYS_CPNT_RSPAN == TRUE)
    UI32_T  rspan_status;
#endif
} VLAN_OM_Dot1qVlanCurrentEntry_T;


typedef struct VLAN_OM_VLIST_T
{
    UI32_T                  vlan_id;
    struct  VLAN_OM_VLIST_T *next;
} VLAN_OM_VLIST_T;

typedef struct
{
    /* dot1qPortVlanEntry defined in RFC 2764 page 65~66
     */

    UI32_T  dot1q_pvid_index;                           /* pvid is stored as pvid_ifindex in vlan_om.*/
    /* The acceptable frame type of this port.
        VAL_dot1qPortAcceptableFrameTypes_admitAll
        VAL_dot1qPortAcceptableFrameTypes_admitOnlyVlanTagged
    */
    UI32_T  dot1q_port_acceptable_frame_types;
    /* Specify if enable the ingress filtering
        VAL_dot1qPortIngressFiltering_true
        VAL_dot1qPortIngressFiltering_false
    */
    UI32_T  dot1q_port_ingress_filtering;
    /* The status of GVRP operation on this port.
        VAL_dot1qGvrpStatus_enabled
        VAL_dot1qGvrpStatus_disabled
    */
    UI32_T  dot1q_port_gvrp_status;
    /* The total number of failed GVRP registrations, for any reason, on this port. */
    UI32_T  dot1q_port_gvrp_failed_registrations;
    /* The Source MAC Address of the last GVRP message received on this port. */
    UI8_T   dot1q_port_gvrp_last_pdu_origin[SIZE_dot1qPortGvrpLastPduOrigin];


    /* These three variables are treated as a backup of dot1q_pvid_index,
       dot1q_port_acceptable_frame_types, and dot1q_port_ingress_filtering,
       because MAC Authentication may change them.
    */
    UI32_T  admin_pvid;
    UI32_T  admin_acceptable_frame_types;
    UI32_T  admin_ingress_filtering;
    /* This item will influence the admin_pvid, admin_acceptable_frame_types,
       and the current_untagged, current_egress in the vlan table
    */
    UI8_T   auto_vlan_mode;

    /* The counter per port indicating the number of static VLANs it joins */
    UI32_T  static_joined_vlan_count;
    /* the number of VLANs it has joined as untagged member */
    UI32_T  untagged_joined_vlan_count;
} VLAN_OM_Dot1qPortVlanEntry_T;

/* Some information about GARP of a port.(See RFC2674 page28)
 */
typedef struct
{
    UI32_T  dot1d_port_garp_join_time;
    UI32_T  dot1d_port_garp_leave_time;
    UI32_T  dot1d_port_garp_leave_all_time;
} VLAN_OM_Dot1dPortGarpEntry_T;

/* ES3626a  Private mib Definition.
 */
typedef struct
{
    /* Indication of Q-Trunk
     * The value of this field is define in leaf_es3626a.h
     * VAL_vlanPortMode_hybrid :    Indicates this port can join both tagged and untagged VLAN
     *                              member set.
     * VAL_vlanPortMode_dot1qTrunk: Indicates this port can only join tagged VLAN member set
     *                              and only transmit and receivd tagged frames.
     * VAL_vlanPortMode_access:     Indicate this port can only join untagged VLAN member set.
     */
    UI32_T  vlan_port_mode;

/* This option is all the ports which are in Q-trunk mode must add to all existent VLAN as tagged.
   So specify the port is in Q-trunk mode or not.
    VLAN_MGR_NOT_TRUNK_LINK
    VLAN_MGR_TRUNK_LINK
*/
#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
    UI32_T  vlan_port_trunk_link_mode;
#endif

#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
    /* Specify whether support dual mode or not. */
    BOOL_T vlan_port_dual_mode;
    /* The VID needed for dual mode port. */
    UI32_T dual_mode_vlan_id;
#endif

    UI16_T  voice_vid;      /* Voice VLAN ID */
    UI16_T  mvr_vid;        /* MVR VLAN ID   */

} VLAN_OM_VlanPortEntry_T;

typedef struct
{
    UI32_T  lport_ifindex;              /* key */
    BOOL_T  port_trunk_mode;            /* Indication of Port-Trunk mode
                                         * TRUE: Indicates if this is a trunk member (dynamic LACP and static)
                                         * FALSE: Indicates this is a normal port
                                         */

    VLAN_OM_Dot1qPortVlanEntry_T    port_item;
    VLAN_OM_Dot1dPortGarpEntry_T    garp_entry;
    VLAN_OM_VlanPortEntry_T         vlan_port_entry;
} VLAN_OM_Vlan_Port_Info_T;


#endif /* #ifndef VLAN_TYPE_H */
