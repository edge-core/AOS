/* Project Name: New Feature
 * File_Name : netaccess_type.h
 * Purpose     : NETACCESS data type definition
 *
 * 2006/01/27    : Ricky Lin  Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (ACP_V3_Enhancement)
 */

#ifndef NETACCESS_TYPE_H
#define NETACCESS_TYPE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_NETACCESS == TRUE)
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "leaf_es3626a.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define NETACCESS_TYPE_SECURE_PORT_APPLY_FILTER_ID_NONE                   0/* NONE *//*move to c file*/
#define NETACCESS_TYPE_MIN_OF_SECURE_MAC_ADDRESS_AUTH_AGE                 10/* in sec */
#define NETACCESS_TYPE_MAX_OF_SECURE_MAC_ADDRESS_AUTH_AGE                 1000000/* in sec */
#define NETACCESS_TYPE_DFLT_SECURE_MAC_ADDRESS_AUTH_AGE                   300/* in sec */
#define NETACCESS_TYPE_DFLT_GUEST_VLAN_ID                                 0

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum NETACCESS_LinkStateChange_E
{
    NETACCESS_PORT_ADMIN_UP     = 1L,
    NETACCESS_PORT_ADMIN_DOWN,
    NETACCESS_PORT_LINK_UP,
    NETACCESS_PORT_LINK_DOWN,
} NETACCESS_LinkStateChange_T;

typedef enum NETACCESS_VlanModified_E
{
    NETACCESS_PORT_ADDED     = 1L,
    NETACCESS_PORT_REMOVED,
} NETACCESS_VlanModified_T;

typedef struct NETACCESS_NEWMAC_DATA_S
{
    UI32_T      lport; /* logical port nbr */
    UI8_T       src_mac[SYS_ADPT_MAC_ADDR_LEN]; /* source MAC address */
    UI32_T      reason; /* reason for new MAC address */
    BOOL_T      is_tag_packet; /* indicate if tag packet or not */
    UI32_T      vid; /* VLAN id */
    void        *cookie;
} NETACCESS_NEWMAC_DATA_T;

typedef struct NETACCESS_EAP_DATA_S
{
    UI8_T       dst_mac[SYS_ADPT_MAC_ADDR_LEN]; /* destination MAC address in eap packet */
    UI8_T       src_mac[SYS_ADPT_MAC_ADDR_LEN]; /* source MAC address in eap packet */
    UI16_T      tag_info;   /* tag_info in eap packet */
    UI16_T      type;       /* packet type */
    UI32_T      pkt_length; /* length of eap packet */
    UI8_T       *pkt_data;  /* pointer for eap packet data */
    UI32_T      lport_no;   /* logical port nbr */
    void        *cookie;
} NETACCESS_EAP_DATA_T;

/* NeedModify:1,need to change MAXSIZE_a3ComPaePortVlanAssignment and MAXSIZE_a3ComPaePortQosAssignment
 */
typedef struct NETACCESS_RADIUS_DATA_S
{
    UI32_T      lport; /* logical port nbr */
    UI8_T       authorized_mac[SYS_ADPT_MAC_ADDR_LEN]; /* authorized MAC address */
    char        authorized_vlan_list[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1]; /* returned VLAN list */
    char        authorized_qos_list[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE + 1]; /* returned QoS list */
    BOOL_T      authorized_result; /* authorized result,succeeded or falied */
    UI32_T      session_time; /* returned session time */
    UI32_T      server_ip;
} NETACCESS_RADIUS_DATA_T;

/* NeedModify:1,need to change MAXSIZE_a3ComPaePortVlanAssignment and MAXSIZE_a3ComPaePortQosAssignment
 */
typedef struct NETACCESS_DOT1X_DATA_S
{
    UI32_T      lport;                                  /* logical port nbr             */
    UI8_T       authorized_mac[SYS_ADPT_MAC_ADDR_LEN];  /* authorized MAC address       */
    UI32_T      eap_identifier;                         /* eap identifier in eap packet */
    char        authorized_vlan_list[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1];  /* returned VLAN list   */
    char        authorized_qos_list[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE + 1]; /* returned QoS list    */
    UI8_T       authorized_result;                      /* authorized result,succeeded or falied */
    UI32_T      session_time;                           /* returned session time        */
    UI32_T      server_ip;
} NETACCESS_DOT1X_DATA_T;

typedef struct NETACCESS_MACAGEOUT_DATA_S
{
    UI32_T      vid;
    UI8_T       mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T      ifindex;
    BOOL_T      is_age;
} NETACCESS_MACAGEOUT_DATA_T;

typedef struct  NETACCESS_NEWMAC_MSGQ_S
{
    NETACCESS_NEWMAC_DATA_T     *m_newmac_data; /* pointer for new MAC data */
    NETACCESS_EAP_DATA_T        *m_eap_data;    /* pointer for eap data */
    UI32_T                      m_reserved[2];  /* not defined for future extension */
} NETACCESS_NEWMAC_MSGQ_T;

typedef struct  NETACCESS_RADIUS_MSGQ_S
{
    NETACCESS_RADIUS_DATA_T     *m_radius_data; /* pointer for radius data */
    UI32_T                      m_reserved[3];  /* not defined for future extension */
} NETACCESS_RADIUS_MSGQ_T;

typedef struct  NETACCESS_DOT1X_MSGQ_S
{
    NETACCESS_DOT1X_DATA_T      *m_dot1x_data; /* pointer for dot1x data */
    UI32_T                      m_reserved[3]; /* not defined for future extension */
} NETACCESS_DOT1X_MSGQ_T;

typedef struct  NETACCESS_MACAGEOUT_MSGQ_S
{
    NETACCESS_MACAGEOUT_DATA_T  *m_macageout_data;
    UI32_T                      m_reserved[3]; /* not defined for future extension */
} NETACCESS_MACAGEOUT_MSGQ_T;

typedef struct  NETACCESS_LinkStateChange_MSGQ_S
{
    NETACCESS_LinkStateChange_T event; /* link state change status */
    UI32_T                      lport; /* logical port nbr */
    UI32_T                      m_reserved[2]; /* not defined for future extension */
} NETACCESS_LinkStateChange_MSGQ_T;

typedef struct  NETACCESS_VlanModified_MSGQ_S
{
    NETACCESS_VlanModified_T    event;  /* VLAN modified status */
    UI32_T                      lport;  /* logical port nbr */
    UI32_T                      vid;    /* VLAN id */
    UI32_T                      status; /* modify status */
} NETACCESS_VlanModified_MSGQ_T;

typedef struct NETACCESS_OM_MacFilterEntry_S
{
    UI8_T   filter_mac[SYS_ADPT_MAC_ADDR_LEN]; /* MAC address */
    UI8_T   filter_mask[SYS_ADPT_MAC_ADDR_LEN]; /* mask */
} NETACCESS_OM_MacFilterEntry_T;

typedef union
{
    NETACCESS_NEWMAC_MSGQ_T           new_mac_msg;
    NETACCESS_RADIUS_MSGQ_T           radius_msg;
    NETACCESS_DOT1X_MSGQ_T            dot1x_msg;
    NETACCESS_LinkStateChange_MSGQ_T  link_change_msg;
    NETACCESS_VlanModified_MSGQ_T     vlan_modified_msg;
    NETACCESS_MACAGEOUT_MSGQ_T        mac_ageout_msg;
} NETACCESS_MSGQ_T;

typedef struct NETWORKACCESS_FilterMacTable_S
{
    UI32_T  mac_index;              /* array index + 1 */
    UI32_T  filter_id;              /* filter id */
    UI8_T   filter_mac[SYS_ADPT_MAC_ADDR_LEN]; /* MAC address */
} NETACCESS_FilterMacTable_T;

typedef struct NETACCESS_PortEapData_S
{
    NETACCESS_EAP_DATA_T    eap_data;

    struct NETACCESS_PortEapData_S  *prev;
    struct NETACCESS_PortEapData_S  *next;

} NETACCESS_PortEapData_T;

typedef struct NETACCESS_PortEapDataList_S
{
    UI32_T  entry_counter;

    NETACCESS_PortEapData_T      *head_of_eap_data_list;
    NETACCESS_PortEapData_T      *tail_of_eap_data_list;

} NETACCESS_PortEapDataList_T;

#if(SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
typedef struct NETACCESS_TYPE_LinkDetectionEntry_S
{
    UI32_T status;
    UI32_T mode;
    UI32_T action;
}NETACCESS_TYPE_LinkDetectionEntry_T;
#endif /* #if(SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE) */


/* for trace_id of user_id when allocate buffer with l_mm
 */
enum
{
    NETACCESS_TYPE_TRACE_ID_NETACCESS_MGR_ANNOUNCENEWMMACCALLBACK = 0,
    NETACCESS_TYPE_TRACE_ID_NETACCESS_MGR_ANNOUNCEEAPPACKETCALLBACK,
    NETACCESS_TYPE_TRACE_ID_NETACCESS_MGR_ANNOUNCERADIUSAUTHORIZEDRESULT,
    NETACCESS_TYPE_TRACE_ID_NETACCESS_MGR_ANNOUNCEDOT1XAUTHORIZEDRESULT,
    NETACCESS_TYPE_TRACE_ID_NETACCESS_MGR_ANNOUNCEMACAGEOUTCALLBACK,
    NETACCESS_TYPE_TRACE_ID_NETACCESS_MGR_MARVELLNACALLBACK,
    NETACCESS_TYPE_TRACE_ID_NETACCESS_VM_LOCALTRIGGERRADADOAUTHENTICATION,
    NETACCESS_TYPE_TRACE_ID_NETACCESS_OM_GETVLANLISTBYSTR,
    NETACCESS_TYPE_TRACE_ID_NETACCESS_OM_EXCLUDEVLANLISTAFROMB,
    NETACCESS_TYPE_TRACE_ID_NETACCESS_OM_EXCLUDEVLANLISTBWHICHNOTINA,
    NETACCESS_TYPE_TRACE_ID_NETACCESS_OM_GETVLANLISTBYPORT,
    NETACCESS_TYPE_TRACE_ID_NETACCESS_OM_DUPLICATEPORTEAPDATA,
    NETACCESS_TYPE_TRACE_ID_NETACCESS_OM_GETFIRSTPORTEAPDATA,
};


enum NETACCESS_EVENT_MASK_E
{
    NETACCESS_EVENT_NONE                =   0x0000L,
    NETACCESS_EVENT_TIMER               =   0x0001L,
    NETACCESS_EVENT_NEWMAC              =   0x0002L,
    NETACCESS_EVENT_RADIUS_RESULT       =   0x0004L,
    NETACCESS_EVENT_DOT1X_RESULT        =   0x0008L,
    NETACCESS_EVENT_ENTER_TRANSITION    =   0x0010L,
    NETACCESS_EVENT_LINK_STATE_CHANGE   =   0x0020L,
    NETACCESS_EVENT_VLAN_MODIFIED       =   0x0040L,
    NETACCESS_EVENT_MAC_AGE_OUT         =   0x0080L,
    NETACCESS_EVENT_ALL                 =   0xFFFFL
};

typedef enum NETACCESS_IntrusionAction_E
{
    NETACCESS_INTRUSIONACTION_NOT_AVAILABLE = 1,
    NETACCESS_INTRUSIONACTION_NO_ACTION,
    NETACCESS_INTRUSIONACTION_DISABLE_PORT,
    NETACCESS_INTRUSIONACTION_DISABLE_PORT_TEMPORARILY,
    NETACCESS_INTRUSIONACTION_ALLOW_DEFAULT_ACCESS,
    NETACCESS_INTRUSIONACTION_TRAP,
    NETACCESS_INTRUSIONACTION_TRAP_AND_DISABLE_PORT,
    NETACCESS_INTRUSIONACTION_TRAP_AND_DISABLE_PORT_TEMPORARILY,

    NETACCESS_INTRUSIONACTION_MAX,
} NETACCESS_IntrusionAction_T;

typedef enum NETACCESS_RowStatus_E
{
    NETACCESS_ROWSTATUS_ACTIVE = 1,
    NETACCESS_ROWSTATUS_NOT_IN_SERVICE,
    NETACCESS_ROWSTATUS_NOT_READY,
    NETACCESS_ROWSTATUS_CREATE_AND_GO,
    NETACCESS_ROWSTATUS_CREATE_AND_WAIT,
    NETACCESS_ROWSTATUS_DESTROY,
} NETACCESS_RowStatus_T;

typedef enum NETACCESS_AuthMode_E
{
    NETACCESS_AUTHMODE_MACADDRESS = 1,
    NETACCESS_AUTHMODE_FIXED,
} NETACCESS_AuthMode_T;

typedef enum NETACCESS_PortMod_E
{
    NETACCESS_PORTMODE_NO_RESTRICTIONS = 1, /* when system boot up, all state machine will stay here */

    NETACCESS_PORTMODE_CONTINUOS_LEARNING,
    NETACCESS_PORTMODE_AUTO_LEARN,
    NETACCESS_PORTMODE_SECURE,

    NETACCESS_PORTMODE_USER_LOGIN,
    NETACCESS_PORTMODE_USER_LOGIN_SECURE,

    NETACCESS_PORTMODE_USER_LOGIN_WITH_OUI,
    NETACCESS_PORTMODE_MAC_ADDRESS_WITH_RADIUS,

    NETACCESS_PORTMODE_MAC_ADDRESS_OR_USER_LOGIN_SECURE,
    NETACCESS_PORTMODE_MAC_ADDRESS_ELSE_USER_LOGIN_SECURE,

    NETACCESS_PORTMODE_MAC_AUTHENTICATION,
    NETACCESS_PORTMODE_PORT_SECURITY,
    NETACCESS_PORTMODE_DOT1X,

    NETACCESS_PORTMODE_DOT1X_SUPPLICANT,
    NETACCESS_PORTMODE_MAX,
} NETACCESS_PortMode_T;

typedef enum NETACCESS_TYPE_MacAuthStatus_E
{
    NETACCESS_TYPE_MACAUTH_ENABLED = VAL_networkAccessPortMode_enabled,
    NETACCESS_TYPE_MACAUTH_DISABLED = VAL_networkAccessPortMode_disabled,
}NETACCESS_TYPE_MacAuthStatus_T;

#define SYS_DFLT_NETACCESS_SECURE_INTRUCTION_ACTION                 NETACCESS_INTRUSIONACTION_NO_ACTION
#define SYS_DFLT_NETACCESS_SECURE_PORT_MODE                         NETACCESS_PORTMODE_NO_RESTRICTIONS

#define SYS_DFLT_NETACCESS_SECURE_AUTH_MODE                         NETACCESS_AUTHMODE_MACADDRESS

#define SYS_DFLT_NETACCESS_MACAUTH_INTRUSIONACTION_ACTION           VAL_macAuthPortIntrusionAction_block_traffic



/* EXPORTED SUBPROGRAM SPECIFICATIONS */

#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
#endif /*NETACCESS_TYPE_H*/
