/* Project Name: New Feature
 * File_Name : networkaccess_type.h
 * Purpose     : NETWORKACCESS data type definition
 *
 * 2004/10/07    : Kevin Cheng  Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (ACP_V3)
 */

#ifndef	NETWORKACCESS_TYPE_H
#define	NETWORKACCESS_TYPE_H
/* INCLUDE FILE DECLARATIONS */
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_dflt.h"

/* NAMING CONSTANT DECLARATIONS */
/* MACRO FUNCTION DECLARATIONS */

/* DATA TYPE DECLARATIONS
 */
/* for trace_id of user_id when allocate buffer with l_mm
 */
enum
{
    NETWORKACCESS_TYPE_TRACE_ID_NETWORKACCESS_MGR_ANNOUNCENEWMAC_CALLBACK = 0,
    NETWORKACCESS_TYPE_TRACE_ID_NETWORKACCESS_MGR_ANNOUNCEEAPPACKET_CALLBACK,
    NETWORKACCESS_TYPE_TRACE_ID_NETWORKACCESS_MGR_ANNOUNCERADAAUTHORIZEDRESULT,
    NETWORKACCESS_TYPE_TRACE_ID_NETWORKACCESS_MGR_ANNOUNCEDOT1XAUTHORIZEDRESULT,
    NETWORKACCESS_TYPE_TRACE_ID_NETWORKACCESS_OM_DUPLICATEPORTEAPDATA,
    NETWORKACCESS_TYPE_TRACE_ID_NETWORKACCESS_OM_GETFIRSTPORTEAPDATA,
    NETWORKACCESS_TYPE_TRACE_ID_NETWORKACCESS_OM_GETVLANLISTBYPORT,
    NETWORKACCESS_TYPE_TRACE_ID_NETWORKACCESS_OM_EXCLUDEVLANLISTAFROMB,
    NETWORKACCESS_TYPE_TRACE_ID_NETWORKACCESS_OM_EXCLUDEVLANLISTBWHICHNOTINA,
    NETWORKACCESS_TYPE_TRACE_ID_NETWORKACCESS_VM_RECALCULATEVLANQOS,
    NETWORKACCESS_TYPE_TRACE_ID_NETWORKACCESS_VM_LOCALTRIGGERRADADOAUTHENTICATION
};

typedef enum NETWORKACCESS_LinkStateChange_E
{
    NETWORKACCESS_PORT_ADMIN_UP     = 1L,
    NETWORKACCESS_PORT_ADMIN_DOWN,
    NETWORKACCESS_PORT_LINK_UP,
    NETWORKACCESS_PORT_LINK_DOWN,
} NETWORKACCESS_LinkStateChange_T;

typedef enum NETWORKACCESS_VlanModified_E
{
    NETWORKACCESS_PORT_ADDED     = 1L,
    NETWORKACCESS_PORT_REMOVED,
} NETWORKACCESS_VlanModified_T;

typedef struct NETWORKACCESS_SecureMacFlag_S
{
    UI8_T   eap_packet              :1; /* eapPacket or not (TRUE - not zero / FALSE - zero) */
    UI8_T   authorized_by_dot1x     :1; /* only one mac SHALL be authorized by 802.1X at any time per port */
    UI8_T   admin_configured_mac    :1; /* administrative configured mac */
    UI8_T   applied_to_chip         :1; /* this mac's setting had been applied to chip */

    UI8_T   write_to_amtr           :1; /* this mac had been written to amtr */
    UI8_T   is_hidden_mac           :1; /* this mac is a hidden MAC or not */
    UI8_T   reserved_bits           :2;
} NETWORKACCESS_SecureMacFlag_T;

typedef struct NETWORKACCESS_SecureMacTable_S
{
    UI32_T  mac_index;              /* array index + 1 */
    UI32_T  slot_index;
    UI32_T  port_index;
    UI8_T   secure_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T  addr_row_status;
    UI32_T  record_time;            /* time stamp that MAC fill in the table */
    UI32_T  authorized_status;      /* authorized(VAL_secureAddrRowStatus_active) or unauthorized(VAL_secureAddrRowStatus_notInService) */
    UI32_T  session_time;           /* an authorized mac reauth period */
    UI32_T  holdoff_time;           /* an unauthorized mac reauth period */
    UI32_T  session_expire_time;    /* reauth time */
    UI32_T  add_on_what_port_mode;  /* record current port mode when MAC is created */
    UI8_T   pae_port_vlan_assignment[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST+1];
    UI8_T   pae_port_qos_assignment[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE+1];

    NETWORKACCESS_SecureMacFlag_T   mac_flag;

} NETWORKACCESS_SecureMacTable_T;

typedef struct NETWORKACCESS_VlanQosCalcResult_S
{
    BOOL_T  is_blocked;     /* if subset is empty, must block the port
                               if there is no returned vlan, use defult config (admin config)
                            */
    UI8_T   vlan_profile[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1];
    UI8_T   qos_profile[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE + 1];
} NETWORKACCESS_VlanQosCalcResult_T;

typedef struct NETWORKACCESS_SecurePortTable_S
{
    UI32_T  slot_index;
    UI32_T  port_index;
    UI32_T  port_mode;
    UI32_T  need_to_know_mode;
    UI32_T  intrusion_action;

    UI32_T  number_addresses;
    UI32_T  configured_number_addresses; /* number_addresses may be increased automatically so need to keep the original value */
    UI32_T  number_addresses_stored;
    UI32_T  maximum_addresses;
    UI32_T  nbr_of_authorized_addresses;
    /* unauthorized mac = number_addresses_stored - nbr_of_authorized_addresses */

    UI32_T  assign_enable;

    UI32_T  trap_of_violation_holdoff_time;

    NETWORKACCESS_VlanQosCalcResult_T   calc_result;

} NETWORKACCESS_SecurePortTable_T;

typedef struct NETWORKACCESS_NEWMAC_DATA_S
{
    UI32_T      lport;
    UI8_T       src_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T      reason;
    BOOL_T      is_tag_packet;
    UI32_T      vid;
} NETWORKACCESS_NEWMAC_DATA_T;

typedef	struct NETWORKACCESS_EAP_DATA_S
{
 	UI8_T		dst_mac[SYS_ADPT_MAC_ADDR_LEN];
	UI8_T		src_mac[SYS_ADPT_MAC_ADDR_LEN];
	UI16_T		tag_info;
 	UI16_T		type;
    UI32_T      pkt_length;
    UI8_T       *pkt_data;
    //UI32_T      unit_no;
    UI32_T      lport_no;
} NETWORKACCESS_EAP_DATA_T;

typedef	struct NETWORKACCESS_RADIUS_DATA_S
{
    UI32_T      lport;
    UI8_T       authorized_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T       authorized_vlan_list[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1];
    UI8_T       authorized_qos_list[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE + 1];
    BOOL_T      authorized_result; /* succeeded or falied */
    UI32_T      session_time;
} NETWORKACCESS_RADIUS_DATA_T;

typedef	struct NETWORKACCESS_DOT1X_DATA_S
{
    UI32_T      lport;
    UI8_T       authorized_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T      eap_identifier;
    UI8_T       authorized_vlan_list[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1];
    UI8_T       authorized_qos_list[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE + 1];
    UI8_T       authorized_result; /* succeeded, failed or logoff */
    UI32_T      session_time;
} NETWORKACCESS_DOT1X_DATA_T;

typedef struct  NETWORKACCESS_NEWMAC_MSGQ_S
{
    NETWORKACCESS_NEWMAC_DATA_T     *m_newmac_data;
    NETWORKACCESS_EAP_DATA_T        *m_eap_data;
    UI32_T                          m_reserved[2]; /* not defined for future extension */
} NETWORKACCESS_NEWMAC_MSGQ_T;

typedef struct  NETWORKACCESS_RADIUS_MSGQ_S
{
    NETWORKACCESS_RADIUS_DATA_T     *m_radius_data;
    UI32_T                          m_reserved[3]; /* not defined for future extension */
} NETWORKACCESS_RADIUS_MSGQ_T;

typedef struct  NETWORKACCESS_DOT1X_MSGQ_S
{
    NETWORKACCESS_DOT1X_DATA_T      *m_dot1x_data;
    UI32_T                          m_reserved[3]; /* not defined for future extension */
} NETWORKACCESS_DOT1X_MSGQ_T;

typedef struct  NETWORKACCESS_LinkStateChange_MSGQ_S
{
    NETWORKACCESS_LinkStateChange_T event;
    UI32_T                          lport;
    UI32_T                          m_reserved[2]; /* not defined for future extension */
} NETWORKACCESS_LinkStateChange_MSGQ_T;

typedef struct  NETWORKACCESS_VlanModified_MSGQ_S
{
    NETWORKACCESS_VlanModified_T    event;
    UI32_T                          lport;
    UI32_T                          vid;
    UI32_T                          status;
} NETWORKACCESS_VlanModified_MSGQ_T;

enum NETWORKACCESS_EVENT_MASK_E
{
    NETWORKACCESS_EVENT_NONE                =   0x0000L,
    NETWORKACCESS_EVENT_TIMER               =   0x0001L,
    NETWORKACCESS_EVENT_NEWMAC              =   0x0002L,
    NETWORKACCESS_EVENT_RADIUS_RESULT       =   0x0004L,
    NETWORKACCESS_EVENT_DOT1X_RESULT        =   0x0008L,
    NETWORKACCESS_EVENT_ENTER_TRANSITION    =   0x0010L,
    NETWORKACCESS_EVENT_LINK_STATE_CHANGE   =   0x0020L,
    NETWORKACCESS_EVENT_VLAN_MODIFIED       =   0x0040L,
    NETWORKACCESS_EVENT_PROVISION_COMPLETE  =   0x0080L,
    NETWORKACCESS_EVENT_ALL                 =   0xFFFFL
};

/* cookie, announce authorized result function pointer */
typedef BOOL_T (*NETWORKACCESS_AuthorizedResultCookie_T)(
                UI32_T lport,
                UI8_T *mac,
                int identifier,
                UI8_T authorized_result,
                UI8_T *authorized_vlan_list,
                UI8_T *authorized_qos_list,
                UI32_T session_time);


/* EXPORTED SUBPROGRAM SPECIFICATIONS */

#endif /*NETWORKACCESS_TYPE_H*/