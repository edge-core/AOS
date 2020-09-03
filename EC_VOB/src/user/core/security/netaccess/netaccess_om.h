/* Project Name: New Feature
 * File_Name : netaccess_om.h
 * Purpose     : NETACCESS database
 *
 * 2006/01/27    : Ricky Lin  Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (ACP_V3_Enhancement)
 */

#ifndef NETACCESS_OM_H
#define NETACCESS_OM_H

/* INCLUDE FILE DECLARATIONS */
#include "sys_type.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_NETACCESS == TRUE)
#include "netaccess_type.h"
#include "sysfun.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define NETACCESS_OM_MAX_HIDDEN_MAC_PER_PORT        1
#define NETACCESS_OM_IPCMSG_TYPE_SIZE           sizeof(union NETACCESS_OM_IpcMsg_Type_U)

/* NeedModify:suggest to move follow declaration to type.h
 */
#define NETACCESS_OM_DEBUG_VM_ERR       0x00000001 /* error */
#define NETACCESS_OM_DEBUG_VM_RST       0x00000002 /* result */
#define NETACCESS_OM_DEBUG_VM_IFO       0x00000004 /* information */
#define NETACCESS_OM_DEBUG_VM_TRC       0x00000008 /* trace */
#define NETACCESS_OM_DEBUG_VM_TMR       0x00000010 /* timer */

#define NETACCESS_OM_DEBUG_OM_ERR       0x00000100 /* error */
#define NETACCESS_OM_DEBUG_OM_IFO       0x00000400 /* information */
#define NETACCESS_OM_DEBUG_OM_TRC       0x00000800 /* trace */

#define NETACCESS_OM_DEBUG_MG_ERR       0x00010000 /* error */
#define NETACCESS_OM_DEBUG_MG_IFO       0x00040000 /* information */
#define NETACCESS_OM_DEBUG_MG_TRC       0x00080000 /* trace */

#define NETACCESS_OM_DEBUG_MODE

#ifdef NETACCESS_OM_DEBUG_MODE
    #define NETACCESS_DBG(op, fmt, args...)                   SECURITY_DEBUG_PRINT(NETACCESS_OM_GetDebugFlag(), op, NETACCESS_OM_GetDebugPrompt(op), fmt, ##args)
    #define NETACCESS_DBG1(op, msg, arg)                      SECURITY_DEBUG_PRINT1(NETACCESS_OM_GetDebugFlag(), op, NETACCESS_OM_GetDebugPrompt(op), msg, arg)
    #define NETACCESS_DBG2(op, msg, arg1, arg2)               SECURITY_DEBUG_PRINT2(NETACCESS_OM_GetDebugFlag(), op, NETACCESS_OM_GetDebugPrompt(op), msg, arg1, arg2)
    #define NETACCESS_DBG3(op, msg, arg1, arg2, arg3)         SECURITY_DEBUG_PRINT3(NETACCESS_OM_GetDebugFlag(), op, NETACCESS_OM_GetDebugPrompt(op), msg, arg1, arg2, arg3)
    #define NETACCESS_DBG4(op, msg, arg1, arg2, arg3, arg4)   SECURITY_DEBUG_PRINT4(NETACCESS_OM_GetDebugFlag(), op, NETACCESS_OM_GetDebugPrompt(op), msg, arg1, arg2, arg3, arg4)
#else
    #define NETACCESS_DBG(op, msg)                            SECURITY_DEBUG_NULL
    #define NETACCESS_DBG1(op, msg, arg)                      SECURITY_DEBUG_NULL
    #define NETACCESS_DBG2(op, msg, arg1, arg2)               SECURITY_DEBUG_NULL
    #define NETACCESS_DBG3(op, msg, arg1, arg2, arg3)         SECURITY_DEBUG_NULL
    #define NETACCESS_DBG4(op, msg, arg1, arg2, arg3, arg4)   SECURITY_DEBUG_NULL
#endif /*NETACCESS_OM_DEBUG_MODE*/


/* command used in IPC message
 */
enum
{
    NETACCESS_OM_IPC_IS_SECURITY_PORT,
};


/* MACRO FUNCTION DECLARATIONS */
/* Macro function for computation of IPC msg_buf size based on field name
 * used in NETACCESS_OM_IpcMsg_T.data
 */
#define NETACCESS_OM_GET_MSG_SIZE(field_name)                        \
            (NETACCESS_OM_IPCMSG_TYPE_SIZE +                         \
            sizeof(((NETACCESS_OM_IpcMsg_T*)0)->data.field_name))

/* DATA TYPE DECLARATIONS
 */
typedef enum NETACCESS_OM_StateMachineStatus_E
{
    NETACCESS_STATE_SYSTEM_INIT = 0, /* when system boot up, all state machine will stay here */

    NETACCESS_STATE_ENTER_SECURE_PORT_MODE,
    NETACCESS_STATE_SECURE_PORT_MODE,
    NETACCESS_STATE_EXIT_SECURE_PORT_MODE,

    NETACCESS_STATE_INIT,
    NETACCESS_STATE_IDLE,

    NETACCESS_STATE_LEARNING,
    NETACCESS_STATE_INTRUSION_HANDLING,

    NETACCESS_STATE_AUTHENTICATING,
    NETACCESS_STATE_SUCCEEDED,
    NETACCESS_STATE_FAILED,

    NETACCESS_STATE_DOT1X_AUTHENTICATING,
    NETACCESS_STATE_RADA_AUTHENTICATING,

    NETACCESS_STATE_DOT1X_FAILED,
    NETACCESS_STATE_RADA_FAILED,

    NETACCESS_STATE_PROTO_UNAWARE,

    NETACCESS_STATE_DISCOVERY,

    NETACCESS_STATE_MAX,

} NETACCESS_OM_StateMachineStatus_T;

typedef struct NETACCESS_OM_HISAMentry_S
{
    UI32_T      lport;                              /* 0, port nbr */
    UI32_T      authorized_status;                  /* 4, authorized status */
    UI32_T      record_time;                        /* 8, time of creation */
    UI32_T      session_expire_time;                /* 12, session expire time to reauth */
    UI32_T      mac_index;                          /* 16, see NETACCESS_OM_SecureMacEntry_T */
    UI8_T       secure_mac[SYS_ADPT_MAC_ADDR_LEN];  /* 20, put at last to avoid alignment issue */
} NETACCESS_OM_HISAMentry_T;

typedef struct NETACCESS_OM_SecureKey_S
{
    UI32_T      lport;                             /* logical port nbr */
    UI8_T       secure_mac[SYS_ADPT_MAC_ADDR_LEN]; /* secure MAC address */
} NETACCESS_OM_SecureKey_T;

typedef struct NETACCESS_OM_OldestKey_S
{
    UI32_T      lport;                             /* logical port nbr */
    UI32_T      authorized_status;                 /* authorized status */
    UI32_T      record_time;                       /* time of creation */
    UI8_T       secure_mac[SYS_ADPT_MAC_ADDR_LEN]; /* secure MAC address */
} NETACCESS_OM_OldestKey_T;

typedef struct NETACCESS_OM_ExpireKey_S
{
    UI32_T      session_expire_time;               /* session expire time to reauth */
    UI32_T      lport;                             /* logical port nbr */
    UI8_T       secure_mac[SYS_ADPT_MAC_ADDR_LEN]; /* secure MAC address */
} NETACCESS_OM_ExpireKey_T;

typedef struct NETACCESS_OM_MacAdrKey_S
{
    UI8_T       secure_mac[SYS_ADPT_MAC_ADDR_LEN]; /* secure MAC address */
    UI32_T      lport;                             /* logical port nbr */
} __attribute__((__packed__)) NETACCESS_OM_MacAdrKey_T;

typedef struct NETACCESS_OM_PortModeChangeSM_S
{
    NETACCESS_OM_StateMachineStatus_T   running_state; /* state machine status */

} NETACCESS_OM_PortModeChangeSM_T;

typedef struct NETACCESS_OM_StateMachineEvent_S
{
    UI32_T new_mac               :1;  /* new mac callback */
    UI32_T eap_packet            :1;  /* eap callback */
    UI32_T reauth                :1;  /* authorized mac session expired */
    UI32_T is_authenticating     :1;  /* enter authenticating phase */
    UI32_T dot1x_logon           :1;  /* already existed a mac authorized by dot1x */
    UI32_T dot1x_success         :1;  /* dot1x authentication success */
    UI32_T dot1x_fail            :1;  /* dot1x authentication fail */
    UI32_T dot1x_logoff          :1;  /* dot1x authentication result: logoff */
    UI32_T dot1x_no_eapol        :1;  /* dot1x authentication result: no receive eapol*/
    UI32_T rada_success          :1;  /* rada authentication success */
    UI32_T rada_fail             :1;  /* rada authentication fail */
    UI32_T waiting_reauth_result :1;  /* whether authentication result belong to reauth or not */
    UI32_T is_tagged             :1;  /* whether new mac is tagged packet */
    UI32_T return_vlan_change    :1;  /* whether return vlan is differ with current */
    UI32_T is_return_vlan_empty  :1;  /* whether return vlan is empty or not */
    UI32_T reserved_bits         :17; /* reserved part */
} NETACCESS_OM_StateMachineEvent_T;

typedef struct NETACCESS_OM_PortSecuritySM_S
{
    NETACCESS_OM_StateMachineStatus_T   running_state; /* running state */
    NETACCESS_OM_StateMachineEvent_T    event_bitmap;  /* event */

    NETACCESS_NEWMAC_MSGQ_T             *new_mac_msg;  /* a duplicate pointer from task */
    NETACCESS_RADIUS_MSGQ_T             *radius_msg;   /* a duplicate pointer from task */
    NETACCESS_DOT1X_MSGQ_T              *dot1x_msg;    /* a duplicate pointer from task */
    NETACCESS_MACAGEOUT_MSGQ_T          *macageout_msg;/* a duplicate pointer from task */

    UI8_T                               authenticating_mac[SYS_ADPT_MAC_ADDR_LEN];  /* src mac */
    UI32_T                              src_vid;
    void                                *cookie;       /* upper layer's cookie, used for sys_callback */
} NETACCESS_OM_PortSecuritySM_T;

typedef struct NETACCESS_OM_StateMachine_S
{
    //UI32_T      lport;                                     /* logical port nbr */
    UI32_T      running_port_mode;                           /* current port mode */
    UI32_T      new_port_mode;                               /* new port mode */

    NETACCESS_OM_PortModeChangeSM_T     port_mode_change_sm; /* mode change state machine */
    NETACCESS_OM_PortSecuritySM_T       port_security_sm;    /* secuirty state machine */

} NETACCESS_OM_StateMachine_T;

/* NeedModify:1,need to change NETACCESS_TYPE_MAX_LEN_OF_VLAN_LIST and NETACCESS_TYPE_MAX_LEN_OF_QOS_PROFILE
 * 2,could merge session_time and holdoff_time ?
 */
typedef struct NETACCESS_OM_VlanQosCalcResult_S
{
    /* because NetAccess does not keep every MAC's VLAN list string,
     * need a flag to remember that the current Dynamic VLAN setting is
     * a port's Original VLAN setting or not.
     */
    UI8_T   is_use_original_vlan_setting        :1;

    /* if subset is empty, must block the port
                             * if there is no returned vlan, use defult config (admin config)
                             */
    UI8_T   is_blocked                          :1;
#if NeeModify
/* not keep vlan information,always get vlan information from vlan_mgr
 */
    UI8_T   vlan_profile[NETACCESS_TYPE_MAX_LEN_OF_VLAN_LIST + 1]; /* vlan result */
#endif
    UI8_T   qos_profile[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE + 1];   /* qos result */
} NETACCESS_OM_VlanQosCalcResult_T;

typedef struct
{
    UI32_T  guest_vlan_id;
    BOOL_T  applied_on_port;
}NETACCESS_OM_GuestVlan_T;

typedef struct
{
    BOOL_T  dynamic_vlan_enabled;
    UI8_T   applied_md5[16];
}NETACCESS_OM_DynamicVlan_T;

typedef struct
{
    BOOL_T  enabled;
    UI8_T   md5[16];
}NETACCESS_OM_DynamicQoS_T;

typedef struct
{
    UI8_T enabled           :1;
    UI8_T detect_linkup     :1;
    UI8_T detect_linkdown   :1;
    UI8_T action_sendtrap   :1;
    UI8_T action_shutdown   :1;
    UI8_T reserved_bits     :3;
}NETACCESS_OM_LinkDetection_T;

typedef struct NETACCESS_OM_SecurePortEntry_S
{
    UI32_T  lport;                       /* key, logical port nbr */
    UI32_T  port_mode;                   /* port mode */
    UI32_T  intrusion_action;            /* action for unauthorized MAC */

    UI32_T  number_addresses;            /* current allowed MAC nbr */
    UI32_T  configured_number_addresses; /* number_addresses may be increased automatically so need to keep the original value */
    UI32_T  number_addresses_stored;     /* current authorized and unauthorized MAC nbr */
    UI32_T  maximum_addresses;           /* maximum MAC nbr */
    UI32_T  nbr_of_authorized_addresses; /* current authorized MAC nbr */
    /* unauthorized mac = number_addresses_stored - nbr_of_authorized_addresses */
    UI32_T  nbr_of_learn_authorized_addresses; /* current authorized MAC nbr */
    UI32_T  filter_id;                   /* filter MAC table id which bind to this port */

    UI32_T  trap_of_violation_holdoff_time; /* holdoff time of violation trap */

    NETACCESS_OM_StateMachine_T         state_machine; /* state machine */
    NETACCESS_OM_VlanQosCalcResult_T    calc_result;   /* current vlan and qos result */

#if (SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
    NETACCESS_OM_LinkDetection_T link_detection;
#endif

#if (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)
    NETACCESS_OM_GuestVlan_T    guest_vlan;
#endif

#if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE) || (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)
    NETACCESS_OM_DynamicVlan_T  dynamic_vlan;
#endif

#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
    NETACCESS_OM_DynamicQoS_T   dynamic_qos;
#endif

} NETACCESS_OM_SecurePortEntry_T;

typedef struct NETACCESS_OM_MacAuthPortEntry_E
{
    UI32_T  lport;                       /* key, logical port nbr */
    UI32_T  intrusion_action;            /* action for unauthorized MAC */
    UI32_T  configured_number_addresses; /* allowed MAC nbr */
}NETACCESS_OM_MacAuthPortEntry_T;

typedef struct NETACCESS_OM_VlanList_S
{
    UI32_T  vid;                           /* VLAN id */
    BOOL_T  is_tagged;                     /* TRUE -- implies tagged */
    BOOL_T  is_pvid;                       /* TRUE -- implies pvid */
    struct NETACCESS_OM_VlanList_S  *prev; /* pointer for previos node */
    struct NETACCESS_OM_VlanList_S  *next; /* pointer for next node */
} NETACCESS_OM_VlanList_T;

typedef struct NETACCESS_OM_SecureMacFlag_S
{
    UI8_T   eap_packet              :1; /* eapPacket or not (TRUE - not zero / FALSE - zero) */
    UI8_T   authorized_by_dot1x     :1; /* only one mac SHALL be authorized by 802.1X at any time per port */
    UI8_T   admin_configured_mac    :1; /* administrative configured mac */
    UI8_T   applied_to_chip         :1; /* this mac's setting had been applied to chip */
    UI8_T   write_to_amtr           :1; /* this mac had been written to amtr */
    UI8_T   is_hidden_mac           :1; /* this mac is a hidden MAC or not */
    UI8_T   is_mac_filter_mac       :1; /* this mac is preauthenticated MAC or not */
    UI8_T   auth_by_rada            :1; /* this mac is authenticated by RADIUS */
} NETACCESS_OM_SecureMacFlag_T;

typedef struct NETACCESS_OM_SecureMacEntry_S
{
    UI32_T  mac_index;              /* key, array index + 1 */
    UI32_T  lport;                  /* logical port nbr */
    UI32_T  addr_row_status;        /* row status */
    UI32_T  record_time;            /* time stamp that MAC fill in the table */
    UI32_T  authorized_status;      /* authorized(VAL_RowStatus_active) or unauthorized(VAL_RowStatus_notInService) */
    UI32_T  session_time;           /* an authorized mac reauth period */
    UI32_T  holdoff_time;           /* an unauthorized mac reauth period */
    UI32_T  session_expire_time;    /* reauth time */
    UI32_T  add_on_what_port_mode;  /* record current port mode when MAC is created */
    UI32_T  vlan_counter;           /* record how many VLAN with this MAC write to AMTR */
    UI32_T  server_ip;              /* authenticated by which server */
    UI8_T   secure_mac[SYS_ADPT_MAC_ADDR_LEN]; /* MAC address */
    NETACCESS_OM_SecureMacFlag_T   mac_flag; /* attribute for this MAC address */
} NETACCESS_OM_SecureMacEntry_T;

typedef struct NETACCESS_OM_StatisticData_S
{
    UI32_T      new_mac_callback_counter;           /* lan intrusion callback counter */
    UI32_T      eap_callback_counter;               /* lan eap callback counter */

    UI32_T      radius_result_cookie_counter;       /* radius mgr pass authentication result via cookie counter */
    UI32_T      dot1x_result_cookie_counter;        /* dot1x mgr pass authentication result via cookie counter */

    UI32_T      demand_radius_auth_counter;         /* demand radius mgr to do authentication counter */
    UI32_T      demand_dot1x_auth_counter;          /* demand dot1x mgr to do authentication counter */

    UI32_T      demand_amtr_set_psec_addr_counter;  /* demand amtr mgr to add psec-learned mac successful counter */
    UI32_T      demand_amtr_delete_addr_counter;    /* demand amtr mgr to delete mac successful counter */

    UI32_T      hidden_mac_used_counter;            /* to know the hidden mac space usage */
} NETACCESS_OM_StatisticData_T;

/* IPC message structure
 */
typedef struct
{
    union NETACCESS_OM_IpcMsg_Type_U
    {
        UI32_T cmd;
        BOOL_T ret_bool;
        UI32_T ret_ui32;
    } type; /* the intended action or return value */

    union
    {
        struct
        {
            UI32_T  lport;
            BOOL_T  bdata;
        } lport_bdata;
    } data;
} NETACCESS_OM_IpcMsg_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetNewMacMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Set id of Msg Q for new mac
 * INPUT:  msgq_id.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetNewMacMsgQId(UI32_T msgq_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetNewMacMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Get id of Msg Q for new mac
 * INPUT:  None.
 * OUTPUT: msgq_id.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetNewMacMsgQId(UI32_T *msgq_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetDot1xMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Set id of Msg Q for dot1x
 * INPUT:  msgq_id.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetDot1xMsgQId(UI32_T msgq_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetDot1xMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Get id of Msg Q for dot1x
 * INPUT:  None.
 * OUTPUT: msgq_id.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetDot1xMsgQId(UI32_T *msgq_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetRadiusMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Set id of Msg Q for RADIUS client
 * INPUT:  msgq_id.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetRadiusMsgQId(UI32_T msgq_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetRadiusMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Get id of Msg Q for RADIUS client
 * INPUT:  None.
 * OUTPUT: msgq_id.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetRadiusMsgQId(UI32_T *msgq_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetLinkStateChangeMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Set id of Msg Q for link state change
 * INPUT:  msgq_id.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetLinkStateChangeMsgQId(UI32_T msgq_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetLinkStateChangeMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Get id of Msg Q for link state change
 * INPUT:  None.
 * OUTPUT: msgq_id.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetLinkStateChangeMsgQId(UI32_T *msgq_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetVlanModifiedMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Set id of Msg Q for vlan modified event
 * INPUT:  msgq_id.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetVlanModifiedMsgQId(UI32_T msgq_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetVlanModifiedMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Get id of Msg Q for vlan modified event
 * INPUT:  None.
 * OUTPUT: msgq_id.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetVlanModifiedMsgQId(UI32_T *msgq_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetMacAgeOutMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Set id of Msg Q for mac age out
 * INPUT:  msgq_id.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetMacAgeOutMsgQId(UI32_T msgq_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetMacAgeOutMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Get id of Msg Q for mac age out
 * INPUT:  None.
 * OUTPUT: msgq_id.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetMacAgeOutMsgQId(UI32_T *msgq_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetTaskId
 * ---------------------------------------------------------------------
 * PURPOSE: Set task id of network access
 * INPUT:  task_id.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetTaskId(UI32_T task_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetTaskId
 * ---------------------------------------------------------------------
 * PURPOSE: Get task id of net access
 * INPUT:  NONE.
 * OUTPUT: task_id.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetTaskId(UI32_T *task_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_InitialSystemResource
 * ---------------------------------------------------------------------
 * PURPOSE: initial om resources (allocate memory, create HASH, HISAM etc)
 * INPUT:  none
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded, FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_InitialSystemResource();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_Initialize
 * ---------------------------------------------------------------------
 * PURPOSE: initialize om
 * INPUT:  none
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded, FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_Initialize();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_ClearAll
 * ---------------------------------------------------------------------
 * PURPOSE: clear om
 * INPUT:  none
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded, FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_ClearAll();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetDot1xAuthorizedResultCookie
 * ---------------------------------------------------------------------
 * PURPOSE: Set authorized result cookie for dot1x
 * INPUT:  cookie
 * OUTPUT: None.
 * RETURN: none
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
void NETACCESS_OM_SetDot1xAuthorizedResultCookie(UI32_T cookie);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetDot1xAuthorizedResultCookie
 * ---------------------------------------------------------------------
 * PURPOSE: get authorized result cookie for dot1x
 * INPUT:  none
 * OUTPUT: cookie
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  if cookie does not exist, return FALSE
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetDot1xAuthorizedResultCookie(UI32_T *cookie);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetRadiusAuthorizedResultCookie
 * ---------------------------------------------------------------------
 * PURPOSE: Set authorized result cookie for radius
 * INPUT:  cookie
 * OUTPUT: None.
 * RETURN: none
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
void NETACCESS_OM_SetRadiusAuthorizedResultCookie(UI32_T cookie);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetRadaAuthorizedResultCookie
 * ---------------------------------------------------------------------
 * PURPOSE: get authorized result cookie for rada
 * INPUT:  none
 * OUTPUT: cookie
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  if cookie does not exist, return FALSE
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetRadaAuthorizedResultCookie(UI32_T *cookie);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetSecurePortMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set security modes of the port
 * INPUT:  lport,secure_port_mode.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  please reference NETACCESS_PortMode_T for secure_port_mode
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecurePortMode(UI32_T lport,NETACCESS_PortMode_T secure_port_mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecurePortMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the learning and security modes of the port
 * INPUT:  lport
 * OUTPUT: secure_port_mode.
 * RETURN: TRUE/FALSE.
 * NOTES:  please reference NETACCESS_PortMode_T for secure_port_mode
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecurePortMode(UI32_T lport, NETACCESS_PortMode_T *secure_port_mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetSecureIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Set the intrusion action to determine the action if an unauthorised device
 *          transmits on this port.
 * INPUT:  lport,secure_intrusion_action.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  please reference NETACCESS_IntrusionAction_T for secure_port_mode
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureIntrusionAction(UI32_T lport,UI32_T secure_intrusion_action);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Get the intrusion action to determine the action if an unauthorised device
 *          transmits on this port.
 * INPUT:  lport
 * OUTPUT: secure_intrusion_action.
 * RETURN: TRUE/FALSE.
 * NOTES:  please reference NETACCESS_IntrusionAction_T for secure_port_mode
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureIntrusionAction(UI32_T lport, UI32_T *secure_intrusion_action);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_SetSecureNumberAddresses
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will set secureNumberAddresses by the unit and the port.
 * INPUT    : lport, number:secureNumberAddresses
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * The maximum number of addresses that the port can learn or
 * store. Reducing this number may cause some addresses to be deleted.
 * This value is set by the user and cannot be automatically changed by the
 * agent.
 *
 * The following relationship must be preserved.
 *
 *    secureNumberAddressesStored <= secureNumberAddresses <=
 *    secureMaximumAddresses.
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureNumberAddresses(UI32_T lport, UI32_T number);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureNumberAddresses
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will get secureNumberAddresses by the unit and the port.
 * INPUT    : unit : secureSlotIndex.
 *            port : securePortIndex.
 * OUTPUT   : number:secureNumberAddresses.
 * RETURN   : TRUE/FALSE
 * NOTES    :
 * The maximum number of addresses that the port can learn or
 * store. Reducing this number may cause some addresses to be deleted.
 * This value is set by the user and cannot be automatically changed by the
 * agent.
 *
 * The following relationship must be preserved.
 *
 *    secureNumberAddressesStored <= secureNumberAddresses <=
 *    secureMaximumAddresses.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureNumberAddresses(UI32_T lport, UI32_T *number);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureNumberAddressesStored
 * ---------------------------------------------------------------------
 * PURPOSE: The number of addresses that are currently in the
 *          AddressTable for this port. If this object has the same value as
 *          secureNumberAddresses, then no more addresses can be authorised on this
 *          port.
 * INPUT:  lport
 * OUTPUT: secure_number_addresses_stored
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureNumberAddressesStored(UI32_T lport, UI32_T *secure_number_addresses_stored);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureNumberAddressesAuthorized
 * ---------------------------------------------------------------------
 * PURPOSE: number of authorized addresses
 * INPUT:  lport
 * OUTPUT: addr_number
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureNumberAddressesAuthorized(UI32_T lport, UI32_T *addr_number);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureNumberAddressesUnauthorized
 * ---------------------------------------------------------------------
 * PURPOSE: number of unauthorized addresses
 * INPUT:  lport
 * OUTPUT: addr_number
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureNumberAddressesUnauthorized(UI32_T lport, UI32_T *addr_number);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureMaximumAddresses
 * ---------------------------------------------------------------------
 * PURPOSE: Get the maxinum value that secureNumberAddresses
 *          can be set to.
 * INPUT:  lport,secure_maximum_addresses.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:
 * This indicates the maximum value that secureNumberAddresses
 * can be set to. It is dependent on the resources available so may change,
 * eg. if resources are shared between ports, then this value can both
 * increase and decrease. This object must be read before setting
 * secureNumberAddresses.
 *
 * The following relationship must allows be preserved.
 *    secureNumberAddressesStored <= secureNumberAddresses <=
 *    secureMaximumAddresses
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureMaximumAddresses(UI32_T lport, UI32_T *secure_maximum_addresses);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_GetHiddenMacNumberByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : get number of hidden mac by lport
 * INPUT    : lport
 * OUTPUT   : qty
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetHiddenMacNumberByPort(UI32_T lport, UI32_T *qty);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureNumberAddressesLearntAuthorized
 * ---------------------------------------------------------------------
 * PURPOSE: number of learnt authorized addresses
 * INPUT:  lport
 * OUTPUT: addr_number
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureNumberAddressesLearntAuthorized(UI32_T lport, UI32_T *addr_number);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_DecreaseSecureNumberAddressesLearntAuthorized
 * ---------------------------------------------------------------------
 * PURPOSE: set number of learnt authorized addresses
 * INPUT:  lport
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_DecreaseSecureNumberAddressesLearntAuthorized(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetTotalReservedSecureAddresses
 * ---------------------------------------------------------------------
 * PURPOSE: Get the total number that secureNumberAddresses had be set.
 * INPUT:  total_reserved_addresses.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetTotalReservedSecureAddresses(UI32_T *total_reserved_addresses);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetMinimumSecureAddresses
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the minimum value that secureNumberAddresses can be set to.
 * INPUT    : port_mode
 * OUTPUT   : min_addresses
 * RETURN   : TRUE -- succeeded / FALSE -- failed.
 * NOTES    : none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetMinimumSecureAddresses(UI32_T port_mode, UI32_T *min_addresses);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetUsableSecureAddresses
 * ---------------------------------------------------------------------
 * PURPOSE: Get the maximum value that secureNumberAddresses can be set to.
 * INPUT:  lport, usable_addresses.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetUsableSecureAddresses(UI32_T lport, UI32_T *usable_addresses);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetUnreservedSecureAddresses
 * ---------------------------------------------------------------------
 * PURPOSE: Get the number of unreserved secure addresses
 * INPUT:   none
 * OUTPUT:  unreserved_nbr
 * RETURN:  TRUE/FALSE.
 * NOTES:   none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetUnreservedSecureAddresses(UI32_T *unreserved_nbr);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_ExtendUpperBoundByPort
 * ---------------------------------------------------------------------
 * PURPOSE: try to increase secureNumberAddresses by 1
 * INPUT:   lport
 * OUTPUT:  none
 * RETURN:  TRUE/FALSE.
 * NOTES:   if there is no unreserved secure address, return FALSE;
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_ExtendUpperBoundByPort(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_ShrinkUpperBoundByPort
 * ---------------------------------------------------------------------
 * PURPOSE: try to decrease secureNumberAddresses by 1
 * INPUT:   lport
 * OUTPUT:  none
 * RETURN:  TRUE/FALSE.
 * NOTES:   if number_addresses <= configured_number_addresses, must check port mode
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_ShrinkUpperBoundByPort(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_IsSecureAddressesFull
 * ---------------------------------------------------------------------
 * PURPOSE: check whether secure address is full or not
 * INPUT:  lport
 * OUTPUT: None.
 * RETURN: TRUE -- full / FALSE -- not yet
 * NOTES:  full ==> secureNumberAddressesStored >= secureNumberAddresses
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_IsSecureAddressesFull(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetSecureReauthTime
 * ---------------------------------------------------------------------
 * PURPOSE: Set the reauth time.
 * INPUT:  reauth_time.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  Specifies the reauth time in seconds before
 *         a forwarding MAC address is re-authenticated.
 *         The default time is 1800 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureReauthTime(UI32_T reauth_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureReauthTime
 * ---------------------------------------------------------------------
 * PURPOSE: Get the reauth time.
 * INPUT:  None.
 * OUTPUT: reauth_time.
 * RETURN: TRUE/FALSE.
 * NOTES:  Specifies the reauth time in seconds before
 *         a forwarding MAC address is re-authenticated.
 *         The default time is 1800 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureReauthTime(UI32_T *reauth_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetSecureAuthAgeTime
 * ---------------------------------------------------------------------
 * PURPOSE: Set the auth age time.
 * INPUT:  auth_age_time.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  Specifies the auth age time in seconds when
 *         a forwarding MAC address will age out.
 *         The default time is 300 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureAuthAgeTime(UI32_T auth_age_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureAuthAgeTime
 * ---------------------------------------------------------------------
 * PURPOSE: Get the auth age time.
 * INPUT:  None.
 * OUTPUT: auth_age_time.
 * RETURN: TRUE/FALSE.
 * NOTES:  Specifies the auth age time in seconds when
 *         a forwarding MAC address will age out.
 *         The default time is 300 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureAuthAgeTime(UI32_T *auth_age_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetSecureHoldoffTime
 * ---------------------------------------------------------------------
 * PURPOSE: Set the holdoff time.
 * INPUT:  holdoff_time.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  Specifies the time in seconds before a blocked (denied)
 *         MAC address can be re-authenticated.
 *         The default time is 60 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureHoldoffTime(UI32_T holdoff_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureHoldoffTime
 * ---------------------------------------------------------------------
 * PURPOSE: Get the holdoff time.
 * INPUT:  None.
 * OUTPUT: holdoff_time.
 * RETURN: TRUE/FALSE.
 * NOTES:  Specifies the time in seconds before a blocked (denied)
 *         MAC address can be re-authenticated.
 *         The default time is 60 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureHoldoffTime(UI32_T *holdoff_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetSecureAuthMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set the authentication mode.
 * INPUT:  auth_mode.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  This controls how MAC addresses are authenticated.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureAuthMode(UI32_T auth_mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureAuthMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the authentication mode.
 * INPUT:  None.
 * OUTPUT: auth_mode.
 * RETURN: TRUE/FALSE.
 * NOTES:  This controls how MAC addresses are authenticated.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureAuthMode(UI32_T *auth_mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetPSecureDynamicVlanStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Set port vlan configuration control
 * INPUT:  lport, dynamic_vlan_status
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES: The user-based port configuration control. Setting this attribute
 *        TRUE causes the port to be configured with any configuration
 *        parameters supplied by the authentication server. Setting this
 *        attribute to FALSE causes any configuration parameters supplied
 *        by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureDynamicVlanStatus(UI32_T lport,BOOL_T dynamic_vlan_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetDynamicVlanStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Get port vlan configuration control
 * INPUT:  lport.
 * OUTPUT: dynamic_vlan_status
 * RETURN: TRUE/FALSE.
 * NOTES: The user-based port configuration control. Setting this attribute
 *        TRUE causes the port to be configured with any configuration
 *        parameters supplied by the authentication server. Setting this
 *        attribute to FALSE causes any configuration parameters supplied
 *        by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetDynamicVlanStatus(UI32_T lport,BOOL_T *dynamic_vlan_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetDynamicVlanMd5
 * ---------------------------------------------------------------------
 * PURPOSE: Set MD5 value of VLAN list string from RADIUS server.
 * INPUT  : lport, md5
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetDynamicVlanMd5(UI32_T lport, UI8_T md5[16]);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_ClearDynamicVlanMd5
 * ---------------------------------------------------------------------
 * PURPOSE: Clear MD5 value of VLAN list string from RADIUS server.
 * INPUT  : lport
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_ClearDynamicVlanMd5(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetDynamicVlanMd5
 * ---------------------------------------------------------------------
 * PURPOSE: Get MD5 value of VLAN list string from RADIUS server.
 * INPUT  : lport.
 * OUTPUT : md5
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetDynamicVlanMd5(UI32_T lport, UI8_T md5[16]);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetSecureDynamicQosStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Set port QoS configuration control
 * INPUT:  lport, dynamic_qos_status
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES: The user-based port configuration control. Setting this attribute
 *        TRUE causes the port to be configured with any configuration
 *        parameters supplied by the authentication server. Setting this
 *        attribute to FALSE causes any configuration parameters supplied
 *        by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureDynamicQosStatus(UI32_T lport,BOOL_T dynamic_qos_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetDynamicQosStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Get port QoS configuration control
 * INPUT:  lport.
 * OUTPUT: dynamic_qos_status
 * RETURN: TRUE/FALSE.
 * NOTES: The user-based port configuration control. Setting this attribute
 *        TRUE causes the port to be configured with any configuration
 *        parameters supplied by the authentication server. Setting this
 *        attribute to FALSE causes any configuration parameters supplied
 *        by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetDynamicQosStatus(UI32_T lport,BOOL_T *dynamic_qos_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetDynamicQosProfileMd5
 * ---------------------------------------------------------------------
 * PURPOSE  : Set md5 of qos profile string from RADIUS server
 * INPUT    : lport     -- lport index
 *            dqos_md5  -- md5 of qos profile string
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetDynamicQosProfileMd5(UI32_T lport, UI8_T dqos_md5[16]);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetDynamicQosProfileMd5
 * ---------------------------------------------------------------------
 * PURPOSE  : Get md5 of qos profile string from RADIUS server
 * INPUT    : lport     -- lport index
 *            dqos_md5  -- md5 of qos profile string
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetDynamicQosProfileMd5(UI32_T lport, UI8_T dqos_md5[16]);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetSecurePortEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will copy secure port entry by the lport.
 * INPUT    : lport
 * OUTPUT   : entry : The secure port entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecurePortEntry(UI32_T lport, NETACCESS_OM_SecurePortEntry_T *entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_CalculateSecureUnitAndPort
 *-----------------------------------------------------------------------------------
 * PURPOSE  : initialize the unit & port of secure port when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port inserted
 *            number_of_port        -- the number of ports on the inserted module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_CalculateSecureUnitAndPort(UI32_T starting_port_ifindex, UI32_T number_of_port);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetSecurePortIsUseOriginalVlan
 * ---------------------------------------------------------------------
 * PURPOSE: set is_use_original_vlan_setting flag of secure port
 * INPUT:  lport, is_blocked
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecurePortIsUseOriginalVlan(UI32_T lport, BOOL_T is_use_original_vlan_setting);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecurePortIsUseOriginalVlan
 * ---------------------------------------------------------------------
 * PURPOSE: get is_use_original_vlan_setting flag of secure port
 * INPUT:  lport.
 * OUTPUT: is_use_original_vlan_setting
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecurePortIsUseOriginalVlan(UI32_T lport, BOOL_T *is_use_original_vlan_setting);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetSecurePortIsBlocked
 * ---------------------------------------------------------------------
 * PURPOSE: set is_block flag of secure port
 * INPUT:  lport, is_blocked
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecurePortIsBlocked(UI32_T lport, BOOL_T is_blocked);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecurePortIsBlocked
 * ---------------------------------------------------------------------
 * PURPOSE: get is_block flag of secure port
 * INPUT:  lport.
 * OUTPUT: is_blocked
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecurePortIsBlocked(UI32_T lport, BOOL_T *is_blocked);

#if NeedModify
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetSecurePortDynamicVlanResult
 * ---------------------------------------------------------------------
 * PURPOSE: set vlan_profile of secure port
 * INPUT:  lport, assignment_result
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecurePortDynamicVlanResult(UI32_T lport, UI8_T *assignment_result);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecurePortDynamicVlanResult
 * ---------------------------------------------------------------------
 * PURPOSE: gt vlan_profile of secure port
 * INPUT:  lport, buffer_size
 * OUTPUT: assignment_result
 * RETURN: TRUE/FALSE.
 * NOTES:  strlen(profile) can't large than buffer_size - 1
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecurePortDynamicVlanResult(UI32_T lport, UI8_T *assignment_result, UI32_T buffer_size);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetNextSecurePortDynamicVlan
 * ---------------------------------------------------------------------
 * PURPOSE: get next dynamic vlan id of secure port
 * INPUT:  lport, vid
 * OUTPUT: vid
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetNextSecurePortDynamicVlan(UI32_T lport, UI32_T *vid);
#endif

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetSecurePortDynamicQosResult
 * ---------------------------------------------------------------------
 * PURPOSE: set qos_profile of secure port
 * INPUT:  lport, assignment_result
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecurePortDynamicQosResult(UI32_T lport, const UI8_T *assignment_result);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecurePortDynamicQosResult
 * ---------------------------------------------------------------------
 * PURPOSE: gt qos_profile of secure port
 * INPUT:  lport, buffer_size
 * OUTPUT: assignment_result
 * RETURN: TRUE/FALSE.
 * NOTES:  strlen(profile) can't large than buffer_size - 1
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecurePortDynamicQosResult(UI32_T lport, UI8_T *assignment_result, UI32_T buffer_size);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_CreateSecureAddressEntry
 * ---------------------------------------------------------------------
 * PURPOSE: create mac entry (om & hisam)
 * INPUT:  mac_entry (key: slot + port + mac)
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  if existed, return FALSE
 *         mac_index, add_on_what_port_mode will be ignore
 *         addr_row_status must be active or notInService
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_CreateSecureAddressEntry(NETACCESS_OM_SecureMacEntry_T *mac_entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_DoesPortAllowCreateSecureAddrEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : whether the specified port allow to create mac or not
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE -- yes / FALSE -- no
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_DoesPortAllowCreateSecureAddrEntry(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_UpdateSecureAddressEntryByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: update mac entry by mac_index (om & hisam)
 * INPUT:  mac_entry
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  if not existed, return FALSE
 *         addr_row_status must be active or notInService
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_UpdateSecureAddressEntryByIndex(NETACCESS_OM_SecureMacEntry_T *mac_entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetSecureAddressEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get secure address entry entry by the unit, port and mac.
 * INPUT    : entry->lport, entry->secure_mac
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureAddressEntry(NETACCESS_OM_SecureMacEntry_T *entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetSecureAddressEntryByIndex
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get secure address entry entry by the unit, port and mac.
 * INPUT    : entry->mac_index
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureAddressEntryByIndex(NETACCESS_OM_SecureMacEntry_T *entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetSecureAddressEntryByMacFlag
 *-----------------------------------------------------------------------------------
 * PURPOSE  : get secure address entry entry by lport and mac_flag
 * INPUT    : entry->lport, entry->mac_flag
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : return the first entry satisfied (entry->mac_flag & om_entry->mac_flag)
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureAddressEntryByMacFlag(NETACCESS_OM_SecureMacEntry_T *entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetNextSecureAddressEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get next secure address entry by the lport and the mac_address.
 * INPUT    : entry->lport, entry->secure_mac
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : get by (lport,MAC) key
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetNextSecureAddressEntry(NETACCESS_OM_SecureMacEntry_T *entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetNextSecureAddressEntryByMacKey
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get next secure address entry by the mac_address and the lport.
 * INPUT    : entry->lport, entry->secure_mac
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : get by (MAC,lport) key
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetNextSecureAddressEntryByMacKey(NETACCESS_OM_SecureMacEntry_T *entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetNextAdminConfiguredSecureAddressEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : get next manual configured secure address entry by the lport and the mac_address.
 * INPUT    : entry->lport, entry->secure_mac
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetNextAdminConfiguredSecureAddressEntry(NETACCESS_OM_SecureMacEntry_T *entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_IsSecureAddressAdminConfigured
 *-----------------------------------------------------------------------------------
 * PURPOSE  : check whether the secure address is administrative configured
 * INPUT    : mac_index
 * OUTPUT   : none
 * RETURN   : TRUE -- admin configured / FALSE -- not admin configured
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_IsSecureAddressAdminConfigured(UI32_T mac_index);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_IsSecureAddressAuthByRadius
 *-----------------------------------------------------------------------------------
 * PURPOSE  : check whether the secure address is authenticated by RADIUS
 * INPUT    : mac_index
 * OUTPUT   : none
 * RETURN   : TRUE -- admin configured / FALSE -- not admin configured
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_IsSecureAddressAuthByRadius(UI32_T mac_index);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_SetSecureAddrRowStatus
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will set secure port entry status by the unit, port and the mac_address.
 * INPUT    : entry->lport, entry->secure_mac, entry->addr_row_status
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : addr_row_status must be active or notInService
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureAddrRowStatus(const NETACCESS_OM_SecureMacEntry_T *entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_SetSecureAddrAppliedToChipByIndex
 *-----------------------------------------------------------------------------------
 * PURPOSE  : set secure port entry applied_to_chip by mac_index
 * INPUT    : mac_index, applied_to_chip
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureAddrAppliedToChipByIndex(UI32_T mac_index, BOOL_T applied_to_chip);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_SetSecureAddrWriteToAmtrByIndex
 *-----------------------------------------------------------------------------------
 * PURPOSE  : set secure port entry write_to_amtr by mac_index
 * INPUT    : mac_index, write_to_amtr
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureAddrWriteToAmtrByIndex(UI32_T mac_index, BOOL_T write_to_amtr);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_DeleteSecureAddressEntryByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: delete mac entry by mac_index (om & hisam)
 * INPUT:  mac_index
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  if not existed, return FALSE
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_DeleteSecureAddressEntryByIndex(UI32_T mac_index);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_DeleteSecureAddressEntryBySecureKey
 * ---------------------------------------------------------------------
 * PURPOSE: delete mac entry by key (om & hisam)
 * INPUT:  key
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  if not existed, return FALSE
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_DeleteSecureAddressEntryBySecureKey(const NETACCESS_OM_SecureKey_T *key);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_DeleteAllSecureAddress
 * ---------------------------------------------------------------------
 * PURPOSE: delete all (authorized & unauthorized) mac addresses from om & hisam
 * INPUT:  none
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_DeleteAllSecureAddress();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_DeleteAllLearnedSecureAddress
 * ---------------------------------------------------------------------
 * PURPOSE: delete all learned mac from om & hisam except manually configured
 * INPUT:  none
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_DeleteAllLearnedSecureAddress(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_DeleteAllSecureAddressByPort
 * ---------------------------------------------------------------------
 * PURPOSE: delete all learned (authorized & unauthorized) mac addresses from om & hisam
 * INPUT:  lport
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_DeleteAllSecureAddressByPort(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_DeleteAllLearnedSecureAddressByPort
 * ---------------------------------------------------------------------
 * PURPOSE: delete all mac addresses from om & hisam except manually configured mac
 * INPUT:  lport
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_DeleteAllLearnedSecureAddressByPort(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetHisamRecordBySecureKey
 *-------------------------------------------------------------------------
 * PURPOSE  : get entry by NETACCESS_OM_SecureKey_T
 * INPUT    : key
 * OUTPUT   : hisam_entry
 * RETURN   : TRUE -- succeeded, FALSE -- failed
 * NOTE     : None
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetHisamRecordBySecureKey(const NETACCESS_OM_SecureKey_T *key, NETACCESS_OM_HISAMentry_T *hisam_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetNextHisamRecordBySecureKey
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next record
 * INPUT    : key
 * OUTPUT   : key, hisam_entry
 * RETURN   : TRUE -- succeeded, FALSE -- failed
 * NOTE     : None
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetNextHisamRecordBySecureKey(NETACCESS_OM_SecureKey_T *key, NETACCESS_OM_HISAMentry_T *hisam_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_DoesRecordExistInHisamBySecureKey
 *-------------------------------------------------------------------------
 * PURPOSE  : check whether the specified record exist or not
 * INPUT    : key
 * OUTPUT   : none
 * RETURN   : TRUE - exist, FALSE - not exist
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_DoesRecordExistInHisamBySecureKey(const NETACCESS_OM_SecureKey_T *key);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetHisamRecordByOldestKey
 *-------------------------------------------------------------------------
 * PURPOSE  : get entry by NETACCESS_OM_OldestKey_T
 * INPUT    : key
 * OUTPUT   : hisam_entry
 * RETURN   : TRUE -- succeeded, FALSE -- failed
 * NOTE     : None
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetHisamRecordByOldestKey(const NETACCESS_OM_OldestKey_T *key, NETACCESS_OM_HISAMentry_T *hisam_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetNextHisamRecordByOldestKey
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next record
 * INPUT    : key
 * OUTPUT   : key, hisam_entry
 * RETURN   : TRUE -- succeeded, FALSE -- failed
 * NOTE     : None
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetNextHisamRecordByOldestKey(NETACCESS_OM_OldestKey_T *key, NETACCESS_OM_HISAMentry_T *hisam_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetHisamRecordByExpireKey
 *-------------------------------------------------------------------------
 * PURPOSE  : get entry by NETACCESS_OM_ExpireKey_T
 * INPUT    : key
 * OUTPUT   : hisam_entry
 * RETURN   : TRUE -- succeeded, FALSE -- failed
 * NOTE     : None
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetHisamRecordByExpireKey(const NETACCESS_OM_ExpireKey_T *key, NETACCESS_OM_HISAMentry_T *hisam_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetNextHisamRecordByExpireKey
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next record
 * INPUT    : key
 * OUTPUT   : key, hisam_entry
 * RETURN   : TRUE -- succeeded, FALSE -- failed
 * NOTE     : None
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetNextHisamRecordByExpireKey(NETACCESS_OM_ExpireKey_T *key, NETACCESS_OM_HISAMentry_T *hisam_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetNextHisamRecordByMacKey
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next record
 * INPUT    : key
 * OUTPUT   : key, hisam_entry
 * RETURN   : TRUE -- succeeded, FALSE -- failed
 * NOTE     : None
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetNextHisamRecordByMacKey(NETACCESS_OM_MacAdrKey_T *key, NETACCESS_OM_HISAMentry_T *hisam_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_SetPortStateMachine
 *-------------------------------------------------------------------------
 * PURPOSE  : set the specific port's state machine
 * INPUT    : lport, state_machine
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_SetPortStateMachine(UI32_T lport, const NETACCESS_OM_StateMachine_T *state_machine);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_GetPortStateMachine
 *-------------------------------------------------------------------------
 * PURPOSE  : get the specific port's state machine
 * INPUT    : lport
 * OUTPUT   : state_machine
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetPortStateMachine(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_GetPortStateMachineRunningPortMode
 *-------------------------------------------------------------------------
 * PURPOSE  : get the specific port's running_port_mode
 * INPUT    : lport
 * OUTPUT   : running_port_mode
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetPortStateMachineRunningPortMode(UI32_T lport, UI32_T *running_port_mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_ClearStateMachineNewMacMsg
 *-------------------------------------------------------------------------
 * PURPOSE  : clear the specific port's new_mac_msg of state machine
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_ClearStateMachineNewMacMsg(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_ClearStateMachineRadiusMsg
 *-------------------------------------------------------------------------
 * PURPOSE  : clear the specific port's radius_msg of state machine
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_ClearStateMachineRadiusMsg(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_ClearStateMachineDot1xMsg
 *-------------------------------------------------------------------------
 * PURPOSE  : clear the specific port's dot1x_msg of state machine
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_ClearStateMachineDot1xMsg(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_ClearStateMachineDot1xLogonFlag
 *-------------------------------------------------------------------------
 * PURPOSE  : clear the specific port's dot1x_logon flag of state machine
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_ClearStateMachineDot1xLogonFlag(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_ClearStateMachineMacAgeOutMsg
 *-------------------------------------------------------------------------
 * PURPOSE  : clear the specific port's macageout_msg of state machine
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_ClearStateMachineMacAgeOutMsg(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_StopStateMachineDoAuthentication
 *-------------------------------------------------------------------------
 * PURPOSE  : state become to idle and turn off is_authenticating flag
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_StopStateMachineDoAuthentication(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_IsStateMachineAuthenticating
 *-------------------------------------------------------------------------
 * PURPOSE  : check whether the specific port's is_authenticating flag of PSEC state machine
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE - yes, FALSE - no
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_IsStateMachineAuthenticating(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_IsThisMacAuthenticatingMac
 *-------------------------------------------------------------------------
 * PURPOSE  : check whether the specific mac on a port is authenticating mac or not
 * INPUT    : lport, mac
 * OUTPUT   : none
 * RETURN   : TRUE - yes, FALSE - no
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_IsThisMacAuthenticatingMac(UI32_T lport, const UI8_T *mac);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_IsThisMacAuthorizedByDot1x
 *-------------------------------------------------------------------------
 * PURPOSE  : check whether the specified (lport, mac) is authorized by dot1x
 * INPUT    : lport, mac
 * OUTPUT   : none
 * RETURN   : TRUE - yes, FALSE - no
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_IsThisMacAuthorizedByDot1x(UI32_T lport, const UI8_T *mac);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_IsThisMacLearnedFromEapPacket
 *-------------------------------------------------------------------------
 * PURPOSE  : check whether the specific mac on a port is learned from an EAP packet or not
 * INPUT    : lport, mac
 * OUTPUT   : none
 * RETURN   : TRUE - yes, FALSE - no
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_IsThisMacLearnedFromEapPacket(UI32_T lport, const UI8_T *mac);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_IsThisMacUnauthorizedMac
 *-------------------------------------------------------------------------
 * PURPOSE  : check whether the specific mac on a port is unauthorized mac or not
 * INPUT    : lport, mac
 * OUTPUT   : none
 * RETURN   : TRUE - yes, FALSE - no
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_IsThisMacUnauthorizedMac(UI32_T lport, const UI8_T *mac);

#if NeedModify
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetVlanListByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : get vlan list by lport
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : if succeeded, return a vlan list; NULL - failed
 * NOTE     : MUST call NETACCESS_OM_FreeVlanList() to free memory
 *-------------------------------------------------------------------------*/
NETACCESS_OM_VlanList_T* NETACCESS_OM_GetVlanListByPort(UI32_T lport);
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetVlanListByStr
 *-------------------------------------------------------------------------
 * PURPOSE  : get vlan list by lport
 * INPUT    : vlan_str
 * OUTPUT   : none
 * RETURN   : if succeeded, return a vlan list; NULL - failed
 * NOTE     : none
 *-------------------------------------------------------------------------*/
NETACCESS_OM_VlanList_T* NETACCESS_OM_GetVlanListByStr(UI8_T *vlan_str);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_FreeVlanList
 *-------------------------------------------------------------------------
 * PURPOSE  : free vlan list
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : none
 * NOTE     : none
 *-------------------------------------------------------------------------*/
void NETACCESS_OM_FreeVlanList(NETACCESS_OM_VlanList_T **vlan_list);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_ExcludeVlanListAFromB
 *-------------------------------------------------------------------------
 * PURPOSE  : exclude the vlans which existed in A from B
 * INPUT    : vlist_a, vlist_b
 * OUTPUT   : vlist_b
 * RETURN   : if succeeded, return a vlan list; NULL - failed
 * NOTE     : e.g. A(1, 2, 3) B(2, 3, 4) ==> (4)
 *-------------------------------------------------------------------------*/
NETACCESS_OM_VlanList_T* NETACCESS_OM_ExcludeVlanListAFromB(const NETACCESS_OM_VlanList_T *vlist_a, const NETACCESS_OM_VlanList_T *vlist_b);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_ExcludeVlanListBWhichNotInA
 *-------------------------------------------------------------------------
 * PURPOSE  : exclude the vlans which doesn't exist in A from B
 * INPUT    : vlist_a, vlist_b
 * OUTPUT   : vlist_b
 * RETURN   : if succeeded, return a vlan list; NULL - failed
 * NOTE     : e.g. A(1, 2, 3) B(2, 3, 4) ==> (2, 3)
 *-------------------------------------------------------------------------*/
NETACCESS_OM_VlanList_T* NETACCESS_OM_ExcludeVlanListBWhichNotInA(const NETACCESS_OM_VlanList_T *vlist_a, const NETACCESS_OM_VlanList_T *vlist_b);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_CompareVlanList
 *-------------------------------------------------------------------------
 * PURPOSE  : compare two vlan list
 * INPUT    : vlist_a, vlist_b
 * OUTPUT   : none
 * RETURN   : return 1 if a > b,
 *            return 0 if a == b,
 *            return -1 if a < b,
 *            return -2 if vid are same but tags are different
 * NOTE     : none
 *-------------------------------------------------------------------------*/
I32_T NETACCESS_OM_CompareVlanList(const NETACCESS_OM_VlanList_T *vlist_a, const NETACCESS_OM_VlanList_T *vlist_b);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_ResetStatisticData
 *-------------------------------------------------------------------------
 * PURPOSE  : reset all counters of statistic data to 0
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETACCESS_OM_ResetStatisticData(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_IncNewMacCounter
 *-------------------------------------------------------------------------
 * PURPOSE  : increase new mac callback counter by 1
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETACCESS_OM_IncNewMacCounter(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_IncEapCounter
 *-------------------------------------------------------------------------
 * PURPOSE  : increase eap callback counter by 1
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETACCESS_OM_IncEapCounter(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_IncRadiusResultCounter
 *-------------------------------------------------------------------------
 * PURPOSE  : increase radius result cookie counter by 1
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETACCESS_OM_IncRadiusResultCounter(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_IncDot1xResultCounter
 *-------------------------------------------------------------------------
 * PURPOSE  : increase dot1x result cookie counter by 1
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETACCESS_OM_IncDot1xResultCounter(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_IncRadiusAuthCounter
 *-------------------------------------------------------------------------
 * PURPOSE  : increase radius authentication counter by 1
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETACCESS_OM_IncRadiusAuthCounter(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_IncDot1xAuthCounter
 *-------------------------------------------------------------------------
 * PURPOSE  : increase dot1x authentication counter by 1
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETACCESS_OM_IncDot1xAuthCounter(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_IncAmtrSetPsecAddrCounter
 *-------------------------------------------------------------------------
 * PURPOSE  : increase amtr set psec address counter by 1
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETACCESS_OM_IncAmtrSetPsecAddrCounter(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_IncAmtrDeleteAddrCounter
 *-------------------------------------------------------------------------
 * PURPOSE  : increase amtr delete address counter by 1
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETACCESS_OM_IncAmtrDeleteAddrCounter(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_GetStatisticData
 *-------------------------------------------------------------------------
 * PURPOSE  : get statistic data
 * INPUT    : none
 * OUTPUT   : data
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetStatisticData(NETACCESS_OM_StatisticData_T *data);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_SetDebugFlag
 *-------------------------------------------------------------------------
 * PURPOSE  : set backdoor debug flag
 * INPUT    : debug_flag
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_SetDebugFlag(UI32_T debug_flag);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_GetDebugFlag
 *-------------------------------------------------------------------------
 * PURPOSE  : get backdoor debug flag
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : debug flag
 * NOTES    : none
 *-------------------------------------------------------------------------*/
UI32_T NETACCESS_OM_GetDebugFlag(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_GetDebugPrompt
 *-------------------------------------------------------------------------
 * PURPOSE  : get the debug prompt
 * INPUT    : flag -- debug flag
 * OUTPUT   : none
 * RETURN   : string form of debug flag
 * NOTES    : none
 *-------------------------------------------------------------------------*/
const char* NETACCESS_OM_GetDebugPrompt(UI32_T flag);

#if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_IsMacFilterExist
 * ---------------------------------------------------------------------
 * PURPOSE: check if mac filter entry exist
 * INPUT:  filter_id,mac_address,mask
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_IsMacFilterExist(UI32_T filter_id, UI8_T *mac_address, UI8_T *mask);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_IsMacFilterMatched
 * ---------------------------------------------------------------------
 * PURPOSE: Check whether for recognised device by mac address.
 * INPUT:   mac_address, mask, unknow_mac
 * OUTPUT:  None.
 * RETURN:  TRUE - match; FALSE - no match
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_IsMacFilterMatched(UI8_T *mac_address, UI8_T *mask, UI8_T *unknow_mac);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetMacFilter
 * ---------------------------------------------------------------------
 * PURPOSE: Set mac filter table
 * INPUT:  filter_id,mac_address,mask,is_add
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetMacFilter(UI32_T filter_id, UI8_T *mac_address, UI8_T *mask, BOOL_T is_add);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_AddMacFilter
 * ---------------------------------------------------------------------
 * PURPOSE: Add mac filter table
 * INPUT:  filter_id,mac_address,mask
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_AddMacFilter(UI32_T filter_id, UI8_T *mac_address, UI8_T *mask);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_DeleteMacFilter
 * ---------------------------------------------------------------------
 * PURPOSE: Delete mac filter table
 * INPUT:  filter_id,mac_address,mask
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_DeleteMacFilter(UI32_T filter_id, UI8_T *mac_address, UI8_T *mask);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetNextMacFilter
 * ---------------------------------------------------------------------
 * PURPOSE: Get next mac filter table entry
 * INPUT:  filter_id,mac_address,mask
 * OUTPUT: filter_id,mac_address,mask
 * RETURN: TRUE/FALSE.
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetNextMacFilter(UI32_T *filter_id, UI8_T *mac_address, UI8_T *mask);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetNextMacFilterByFilterId
 * ---------------------------------------------------------------------
 * PURPOSE: Get the next MAC filter entry that have the same filter ID
 * INPUT:  filter_id,mac_address,mask
 * OUTPUT: mac_address,mask
 * RETURN: TRUE/FALSE.
 * NOTES:  Using zero mac_address to get the first MAC filter entry
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetNextMacFilterByFilterId(UI32_T filter_id, UI8_T *mac_address, UI8_T *mask);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetApplyMacFilterByPort
 *-----------------------------------------------------------------------------------
 * PURPOSE  : Get mac filter id which bind to port
 * INPUT    : ifindex
 * OUTPUT   : filter_id.
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetApplyMacFilterByPort(UI32_T ifindex, UI32_T *filter_id);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_SetApplyMacFilterByPort
 *-----------------------------------------------------------------------------------
 * PURPOSE  : bind mac filter id to port
 * INPUT    : ifindex,filter_id
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetApplyMacFilterByPort(UI32_T ifindex, UI32_T filter_id);
#endif /* #if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE) */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_OM_InitUnauthorizedMacCache
 * -------------------------------------------------------------------------
 * PURPOSE: Initial database for Unauthorized MAC cache.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_InitUnauthorizedMacCache(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_OM_ClearUnauthorizedMacCache
 * -------------------------------------------------------------------------
 * PURPOSE: Clear Unauthorized MAC cache.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_ClearUnauthorizedMacCache (void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_OM_AddOneMac2UnauthorizedMacCache
 * -------------------------------------------------------------------------
 * PURPOSE: Add One MAC to Unauthorized MAC cache.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_AddOneMac2UnauthorizedMacCache (UI8_T *src_mac_p);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_OM_IsMacExistInUnauthorizedMacCache
 * -------------------------------------------------------------------------
 * PURPOSE: Check if one MAC is exist in Unauthorized MAC cache or not.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_IsMacExistInUnauthorizedMacCache (UI8_T *src_mac_p);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_OM_GetUnauthorizedMacCacheExpireTime
 * -------------------------------------------------------------------------
 * PURPOSE: get Unauthorized MAC expire time.
 * INPUT  : expire_time_p.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetUnauthorizedMacCacheExpireTime(UI32_T *expire_time_p);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_OM_SetUnauthorizedMacCacheExpireTime
 * -------------------------------------------------------------------------
 * PURPOSE: set Unauthorized MAC expire time.
 * INPUT  : expire_time.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------*/
void NETACCESS_OM_SetUnauthorizedMacCacheExpireTime(UI32_T expire_time);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_OM_SetSecureAddrVidCounterByIndex
 * -------------------------------------------------------------------------
 * PURPOSE: set authenticated MAC address VLAN counter.
 * INPUT  : mac_index, vid_counter.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureAddrVidCounterByIndex(UI32_T mac_index, UI32_T vid_counter);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_OM_GetSecureAddrVidCounterByIndex
 * -------------------------------------------------------------------------
 * PURPOSE: get authenticated MAC address VLAN counter.
 * INPUT  : mac_index.
 * OUTPUT : vid_counter.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureAddrVidCounterByIndex(UI32_T mac_index, UI32_T *vid_counter);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_OM_IncSecureAddrVidCounterByIndex
 * -------------------------------------------------------------------------
 * PURPOSE: increase authenticated MAC address VLAN counter.
 * INPUT  : mac_index, vid_counter.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_IncSecureAddrVidCounterByIndex(UI32_T mac_index);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_OM_DecSecureAddrVidCounterByIndex
 * -------------------------------------------------------------------------
 * PURPOSE: decrease authenticated MAC address VLAN counter.
 * INPUT  : mac_index, vid_counter.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_DecSecureAddrVidCounterByIndex(UI32_T mac_index);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETACCESS_OM_DecreaseSecureAddress
 * -------------------------------------------------------------------------
 * PURPOSE: decrease authenticated MAC address.
 * INPUT  : key
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_DecreaseSecureAddress(NETACCESS_OM_SecureKey_T *key);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_CreatePortDisableTimer
 *-------------------------------------------------------------------------
 * PURPOSE  : create disablePortTemporarily timer
 * INPUT    : lport, expire_time
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_CreatePortDisableTimer(UI32_T lport, UI32_T expire_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_DestroyFirstPortDisableTimer
 *-------------------------------------------------------------------------
 * PURPOSE  : destroy disablePortTemporarily timer
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETACCESS_OM_DestroyFirstPortDisableTimer();

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_GetFirstPortDisableTimer
 *-------------------------------------------------------------------------
 * PURPOSE  : get first disablePortTemporarily timer
 * INPUT    : none
 * OUTPUT   : lport, expire_time
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetFirstPortDisableTimer(UI32_T *lport, UI32_T *expire_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetSecureViolationTrapHoldoffTime
 * ---------------------------------------------------------------------
 * PURPOSE: Set the violation trap hold off time on this port.
 * INPUT:  lport, holdoff_time
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureViolationTrapHoldoffTime(UI32_T lport, UI32_T holdoff_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureViolationTrapHoldoffTime
 * ---------------------------------------------------------------------
 * PURPOSE: get the violation trap hold off time on this port.
 * INPUT:  lport
 * OUTPUT: holdoff_time
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureViolationTrapHoldoffTime(UI32_T lport, UI32_T *holdoff_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetSecureGuestVlanId
 * ---------------------------------------------------------------------
 * PURPOSE  : Set guest VLAN ID of the port
 * INPUT    : lport,vid.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    : The range of guest VLAN ID is from 0 to MAX_networkAccessPortGuestVlan,
 *            and 0 means guest VLAN feature is disabled.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetSecureGuestVlanId(UI32_T lport, UI32_T vid);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetSecureGuestVlanId
 * ---------------------------------------------------------------------
 * PURPOSE  : Get guest VLAN ID of the port
 * INPUT    : lport.
 * OUTPUT   : vid.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetSecureGuestVlanId(UI32_T lport, UI32_T *vid);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetJoinRestrictedVlanStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : Set join restricted VLAN status of the port
 * INPUT    : lport  -- lport index
 *            status -- TRUE, the port join to restricted VLAN/FALSE, no.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetJoinRestrictedVlanStatus(UI32_T lport, BOOL_T status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetJoinRestrictedVlanStatus
 * ---------------------------------------------------------------------
 * PURPOSE  : Get join restricted VLAN status of the port
 * INPUT    : lport  -- lport index
 * OUTPUT   : status -- TRUE, the port join to restricted VLAN/FALSE, no.
 * RETURN   : TRUE/FALSE.
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetJoinRestrictedVlanStatus(UI32_T lport, BOOL_T *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_DuplicatePortEapData
 *-------------------------------------------------------------------------
 * PURPOSE  : duplicate eap data in om by lport
 * INPUT    : lport, eap_data
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_DuplicatePortEapData(UI32_T lport, const NETACCESS_EAP_DATA_T *eap_data);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_DestroyFirstPortEapData
 *-------------------------------------------------------------------------
 * PURPOSE  : destroy the first eap data entry by lport
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETACCESS_OM_DestroyFirstPortEapPacket(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_DestroyAllPortEapData
 *-------------------------------------------------------------------------
 * PURPOSE  : destroy all eap data entry by lport
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETACCESS_OM_DestroyAllPortEapData(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_GetFirstPortEapData
 *-------------------------------------------------------------------------
 * PURPOSE  : get the first eap data by lport
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : if succeeded, return a eap data; NULL - failed
 * NOTES    : MUST call NETACCESS_OM_FreeEapData() to free memory
 *-------------------------------------------------------------------------*/
NETACCESS_EAP_DATA_T *NETACCESS_OM_GetFirstPortEapData(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_FreeEapData
 *-------------------------------------------------------------------------
 * PURPOSE  : free eap data
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : none
 * NOTE     : none
 *-------------------------------------------------------------------------*/
void NETACCESS_OM_FreeEapData(NETACCESS_EAP_DATA_T **eap_data);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_SetMacAuthPortMaxMacCount
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the max allowed MAC number of mac-authentication for the
 *            specified port.
 * INPUT    : lport -- logic port number
 *            count -- max mac count
 * OUTPUT   : None.
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : None.
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_SetMacAuthPortMaxMacCount(UI32_T lport, UI32_T count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_GetMacAuthPortMaxMacCount
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the max allowed MAC number of mac-authentication for the
 *            specified port.
 * INPUT    : lport -- logic port number
 * OUTPUT   : count -- max mac count
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetMacAuthPortMaxMacCount(UI32_T lport, UI32_T *count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_SetMacAuthPortIntrusionAction
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the intrustion action of mac-authentication for the specified port.
 * INPUT    : lport  -- logic port number
 *            action -- intrusion action
 * OUTPUT   : none.
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : intrusion action
 *            VAL_macAuthPortIntrusionAction_block_traffic for block action
 *            VAL_macAuthPortIntrusionAction_pass_traffic for pass action
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_SetMacAuthPortIntrusionAction(UI32_T lport, UI32_T action);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_GetMacAuthPortIntrusionAction
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the intrusion action of mac-authentication for the specified port.
 * INPUT    : lport -- logic port number
 * OUTPUT   : action -- intrusion action
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : intrusion action
 *            VAL_macAuthPortIntrusionAction_block_traffic for block action
 *            VAL_macAuthPortIntrusionAction_pass_traffic for pass action
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_OM_GetMacAuthPortIntrusionAction(UI32_T lport, UI32_T *action);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_GetMacAuthenticationEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : Get the mac authentication entry by the lport.
 * INPUT    : lport
 * OUTPUT   : entry : The mac-authentication port entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetMacAuthPortEntry(UI32_T lport, NETACCESS_OM_MacAuthPortEntry_T *entry);

#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_SetMacAddressAgingMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the MAC address aging mode.
 * INPUT    : mode -- MAC address aging mode
 * OUTPUT   : None.
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : Aging mode
 *            VAL_networkAccessAging_enabled for aging enabled
 *            VAL_networkAccessAging_disabled for aging disabled
 *-------------------------------------------------------------------------
*/
BOOL_T NETACCESS_OM_SetMacAddressAgingMode(UI32_T mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_OM_GetMacAddressAgingMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the MAC address aging mode.
 * INPUT    : None.
 * OUTPUT   : mode_p -- MAC address aging mode
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : Aging mode
 *            VAL_networkAccessAging_enabled for aging enabled
 *            VAL_networkAccessAging_disabled for aging disabled
 *-------------------------------------------------------------------------
*/
BOOL_T NETACCESS_OM_GetMacAddressAgingMode(UI32_T *mode_p);
#endif /*#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)*/

#if(SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetLinkDetectionStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Get port link detection status.
 * INPUT  : None.
 * OUTPUT : status_p -- VAL_networkAccessPortLinkDetectionStatus_enabled
 *                    VAL_networkAccessPortLinkDetectionStatus_disabled
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetLinkDetectionStatus(UI32_T lport, UI32_T *status_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetLinkDetectionStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Set port link detection status.
 * INPUT  : status -- VAL_networkAccessPortIntrusionLockStatus_enabled
 *                    VAL_networkAccessPortIntrusionLockStatus_disable
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetLinkDetectionStatus(UI32_T lport, UI32_T status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetLinkDetectionMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get port  link detection mode.
 * INPUT  : None.
 * OUTPUT  : mode_p -- VAL_networkAccessPortLinkDetectionMode_linkUp
 *                    VAL_networkAccessPortLinkDetectionMode_linkDown
 *                    VAL_networkAccessPortLinkDetectionMode_linkUpDown
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetLinkDetectionMode(UI32_T lport, UI32_T *mode_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetLinkDetectionMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set port link detection mode.
 * INPUT  : mode -- VAL_networkAccessPortLinkDetectionMode_linkUp
 *                    VAL_networkAccessPortLinkDetectionMode_linkDown
 *                    VAL_networkAccessPortLinkDetectionMode_linkUpDown
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetLinkDetectionMode(UI32_T lport, UI32_T mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_GetLinkDetectionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Get port  link detection action.
 * INPUT  : None.
 * OUTPUT  : action_p -- VAL_networkAccessPortLinkDetectionAciton_trap
 *                     VAL_networkAccessPortLinkDetectionAciton_shutdown
 *                     VAL_networkAccessPortLinkDetectionAciton_trapAndShutdown
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_GetLinkDetectionAction(UI32_T lport, UI32_T *action_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_SetLinkDetectionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Get port  link detection action.
 * INPUT  : None.
 * OUTPUT  : action -- VAL_networkAccessPortLinkDetectionAciton_trap
 *                     VAL_networkAccessPortLinkDetectionAciton_shutdown
 *                     VAL_networkAccessPortLinkDetectionAciton_trapAndShutdown
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_SetLinkDetectionAction(UI32_T lport, UI32_T action);
#endif /* #if(SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE) */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_IsNeedMutualExclusiveCheck
 * ---------------------------------------------------------------------
 * PURPOSE: Check if port mode need to do mutual exclusive checking
 *          with other CSC
 * INPUT:   port_mode
 * OUTPUT:  none
 * RETURN:  TRUE/FALSE
 * NOTES:   none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_IsNeedMutualExclusiveCheck(UI32_T port_mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_IsSecurityPort
 * ---------------------------------------------------------------------
 * PURPOSE: Check if security related function is enabled on lport
 * INPUT:   lport.
 * OUTPUT:  is_enabled.
 * RETURN:  TRUE/FALSE
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_IsSecurityPort(UI32_T lport, BOOL_T *is_enabled);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_OM_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for NETACCESS OM.
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
BOOL_T NETACCESS_OM_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_OM_UpdateCookie
 * ---------------------------------------------------------------------
 * PURPOSE: To transmit cookie to next CSC.
 * INPUT  : lport -- port number
 *          cookie_p -- callback data
 * OUTPUT : None.
 * RETURN : TRUE  -- succeeded
 *          FALSE -- failed
 * NOTES :
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_OM_UpdateCookie(
  UI32_T lport,
  void *cookie_p);

#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
#endif /*NETACCESS_OM_H*/
