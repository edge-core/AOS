/* Project Name: New Feature
 * File_Name : networkaccess_om.h
 * Purpose     : NETWORKACCESS database
 *
 * 2004/10/11    : Kevin Cheng  Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (ACP_V3)
 */
 
#ifndef	NETWORKACCESS_OM_H
#define	NETWORKACCESS_OM_H

/* INCLUDE FILE DECLARATIONS */
#include "sys_type.h" 
#include "networkaccess_type.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define NETWORKACCESS_OM_MAX_HIDDEN_MAC_PER_PORT        1

#define NETWORKACCESS_OM_DEBUG_VM_ERR       0x00000001 /* error */
#define NETWORKACCESS_OM_DEBUG_VM_RST       0x00000002 /* result */
#define NETWORKACCESS_OM_DEBUG_VM_IFO       0x00000004 /* information */
#define NETWORKACCESS_OM_DEBUG_VM_TRC       0x00000008 /* trace */
#define NETWORKACCESS_OM_DEBUG_VM_TMR       0x00000010 /* timer */

#define NETWORKACCESS_OM_DEBUG_OM_ERR       0x00000100
#define NETWORKACCESS_OM_DEBUG_OM_IFO       0x00000400
#define NETWORKACCESS_OM_DEBUG_OM_TRC       0x00000800

#define NETWORKACCESS_OM_DEBUG_MG_ERR       0x00010000
#define NETWORKACCESS_OM_DEBUG_MG_IFO       0x00040000
#define NETWORKACCESS_OM_DEBUG_MG_TRC       0x00080000


/* MACRO FUNCTION DECLARATIONS */

/* DATA TYPE DECLARATIONS
 */
typedef enum NETWORKACCESS_OM_StateMachineStatus_E
{
    NETWORKACCESS_STATE_SYSTEM_INIT = 0, /* when system boot up, all state machine will stay here */

    NETWORKACCESS_STATE_ENTER_SECURE_PORT_MODE,
    NETWORKACCESS_STATE_SECURE_PORT_MODE,
    NETWORKACCESS_STATE_EXIT_SECURE_PORT_MODE,

    NETWORKACCESS_STATE_INIT,
    NETWORKACCESS_STATE_IDLE,

    NETWORKACCESS_STATE_LEARNING,
    NETWORKACCESS_STATE_INTRUSION_HANDLING,

    NETWORKACCESS_STATE_AUTHENTICATING,
    NETWORKACCESS_STATE_SUCCEEDED,
    NETWORKACCESS_STATE_FAILED,

    NETWORKACCESS_STATE_DOT1X_AUTHENTICATING,
    NETWORKACCESS_STATE_RADA_AUTHENTICATING,

    NETWORKACCESS_STATE_DOT1X_FAILED,
    NETWORKACCESS_STATE_RADA_FAILED,

    NETWORKACCESS_STATE_,

} NETWORKACCESS_OM_StateMachineStatus_T;

typedef struct NETWORKACCESS_OM_HISAMentry_S
{
    UI32_T      slot_index;                         /* 0 */
    UI32_T      port_index;                         /* 4 */
    UI32_T      authorized_status;                  /* 8 */
    UI32_T      record_time;                        /* 12 */
    UI32_T      session_expire_time;                /* 16 */
    UI32_T      mac_index;                          /* 20, see NETWORKACCESS_SecureMacTable_T */
    UI8_T       secure_mac[SYS_ADPT_MAC_ADDR_LEN];  /* 24, put at last to avoid alignment issue */
} NETWORKACCESS_OM_HISAMentry_T;

typedef struct NETWORKACCESS_OM_SecureKey_S
{
    UI32_T      slot_index;
    UI32_T      port_index;
    UI8_T       secure_mac[SYS_ADPT_MAC_ADDR_LEN];
} NETWORKACCESS_OM_SecureKey_T;

typedef struct NETWORKACCESS_OM_OldestKey_S
{
    UI32_T      slot_index;
    UI32_T      port_index;
    UI32_T      authorized_status;
    UI32_T      record_time;
    UI8_T       secure_mac[SYS_ADPT_MAC_ADDR_LEN];
} NETWORKACCESS_OM_OldestKey_T;

typedef struct NETWORKACCESS_OM_ExpireKey_S
{
    UI32_T      session_expire_time;
    UI32_T      slot_index;
    UI32_T      port_index;
    UI8_T       secure_mac[SYS_ADPT_MAC_ADDR_LEN];
} NETWORKACCESS_OM_ExpireKey_T;

typedef struct NETWORKACCESS_OM_MacAdrKey_S
{
    UI8_T       secure_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T      slot_index;
    UI32_T      port_index;
} NETWORKACCESS_OM_MacAdrKey_T __attribute__ ((packed));

typedef struct NETWORKACCESS_OM_PortModeChangeSM_S
{
    NETWORKACCESS_OM_StateMachineStatus_T   running_state;

} NETWORKACCESS_OM_PortModeChangeSM_T;

typedef struct NETWORKACCESS_OM_StateMachineEvent_S
{
    UI32_T      new_mac                 :1;     /* new mac callback */
    UI32_T      eap_packet              :1;     /* eap callback */
    UI32_T      reauth                  :1;     /* authorized mac session expired */
    UI32_T      is_authenticating       :1;     /* enter authenticating phase */
    UI32_T      dot1x_logon             :1;     /* already existed a mac authorized by dot1x */

    UI32_T      dot1x_success           :1;     /* dot1x authentication result: success */
    UI32_T      dot1x_fail              :1;     /* dot1x authentication result: fail */
    UI32_T      dot1x_logoff            :1;     /* dot1x authentication result: logoff */
    UI32_T      rada_success            :1;     /* rada authentication success */
    UI32_T      rada_fail               :1;     /* rada authentication fail */

    UI32_T      waiting_reauth_result   :1;     /* whether authentication result belong to reauth or not */

    UI32_T      reserved_bits           :21;
} NETWORKACCESS_OM_StateMachineEvent_T;

typedef struct NETWORKACCESS_OM_PortSecuritySM_S
{
    NETWORKACCESS_OM_StateMachineStatus_T   running_state;
    NETWORKACCESS_OM_StateMachineEvent_T    event_bitmap;

    NETWORKACCESS_NEWMAC_MSGQ_T             *new_mac_msg;   /* a duplicate pointer from task */
    NETWORKACCESS_RADIUS_MSGQ_T             *radius_msg;    /* a duplicate pointer from task */
    NETWORKACCESS_DOT1X_MSGQ_T              *dot1x_msg;     /* a duplicate pointer from task */

    UI8_T       authenticating_mac[SYS_ADPT_MAC_ADDR_LEN];  /* src mac */

} NETWORKACCESS_OM_PortSecuritySM_T;

typedef struct NETWORKACCESS_OM_StateMachine_S
{
    UI32_T      lport;
    UI32_T      running_port_mode;
    UI32_T      new_port_mode;

    NETWORKACCESS_OM_PortModeChangeSM_T     port_mode_change_sm;
    NETWORKACCESS_OM_PortSecuritySM_T       port_security_sm;

} NETWORKACCESS_OM_StateMachine_T;

typedef struct NETWORKACCESS_OM_VlanList_S
{
    UI32_T  vid;
    struct NETWORKACCESS_OM_VlanList_S  *prev;
    struct NETWORKACCESS_OM_VlanList_S  *next;
} NETWORKACCESS_OM_VlanList_T;

typedef struct NETWORKACCESS_OM_StatisticData_S
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
} NETWORKACCESS_OM_StatisticData_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_SetNewMacMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Set id of Msg Q for new mac
 * INPUT:  msgq_id.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_SetNewMacMsgQId(UI32_T msgq_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetNewMacMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Get id of Msg Q for new mac
 * INPUT:  None.
 * OUTPUT: msgq_id.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetNewMacMsgQId(UI32_T *msgq_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_SetDot1xMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Set id of Msg Q for dot1x
 * INPUT:  msgq_id.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_SetDot1xMsgQId(UI32_T msgq_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetDot1xMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Get id of Msg Q for dot1x
 * INPUT:  None.
 * OUTPUT: msgq_id.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetDot1xMsgQId(UI32_T *msgq_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_SetRadiusMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Set id of Msg Q for RADIUS client
 * INPUT:  msgq_id.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_SetRadiusMsgQId(UI32_T msgq_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetRadiusMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Get id of Msg Q for RADIUS client
 * INPUT:  None.
 * OUTPUT: msgq_id.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetRadiusMsgQId(UI32_T *msgq_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_SetLinkStateChangeMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Set id of Msg Q for link state change
 * INPUT:  msgq_id.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_SetLinkStateChangeMsgQId(UI32_T msgq_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetLinkStateChangeMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Get id of Msg Q for link state change
 * INPUT:  None.
 * OUTPUT: msgq_id.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetLinkStateChangeMsgQId(UI32_T *msgq_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_SetVlanModifiedMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Set id of Msg Q for vlan modified event
 * INPUT:  msgq_id.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_SetVlanModifiedMsgQId(UI32_T msgq_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetVlanModifiedMsgQId
 * ---------------------------------------------------------------------
 * PURPOSE: Get id of Msg Q for vlan modified event
 * INPUT:  None.
 * OUTPUT: msgq_id.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetVlanModifiedMsgQId(UI32_T *msgq_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_SetTaskId
 * ---------------------------------------------------------------------
 * PURPOSE: Set task id of network access
 * INPUT:  task_id.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_SetTaskId(UI32_T task_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetTaskId
 * ---------------------------------------------------------------------
 * PURPOSE: Get task id of network access
 * INPUT:  NONE.
 * OUTPUT: task_id.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetTaskId(UI32_T *task_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_Initialize
 * ---------------------------------------------------------------------
 * PURPOSE: initialize om
 * INPUT:  none
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded, FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_Initialize();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_SetDot1xAuthorizedResultCookie
 * ---------------------------------------------------------------------
 * PURPOSE: Set authorized result cookie for dot1x
 * INPUT:  cookie
 * OUTPUT: None.
 * RETURN: none
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
void NETWORKACCESS_OM_SetDot1xAuthorizedResultCookie(NETWORKACCESS_AuthorizedResultCookie_T cookie);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetDot1xAuthorizedResultCookie
 * ---------------------------------------------------------------------
 * PURPOSE: get authorized result cookie for dot1x
 * INPUT:  none
 * OUTPUT: cookie
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  if cookie does not exist, return FALSE
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetDot1xAuthorizedResultCookie(NETWORKACCESS_AuthorizedResultCookie_T *cookie);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_SetRadaAuthorizedResultCookie
 * ---------------------------------------------------------------------
 * PURPOSE: Set authorized result cookie for rada
 * INPUT:  cookie
 * OUTPUT: None.
 * RETURN: none
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
void NETWORKACCESS_OM_SetRadaAuthorizedResultCookie(NETWORKACCESS_AuthorizedResultCookie_T cookie);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetRadaAuthorizedResultCookie
 * ---------------------------------------------------------------------
 * PURPOSE: get authorized result cookie for rada
 * INPUT:  none
 * OUTPUT: cookie
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  if cookie does not exist, return FALSE
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetRadaAuthorizedResultCookie(NETWORKACCESS_AuthorizedResultCookie_T *cookie);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_SetSecurePortMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set the learning and security modes of the port 
 * INPUT:  lport,secure_port_mode.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  secure_port_mode = 
 *         VAL_securePortMode_noRestrictions	1L
 *         VAL_securePortMode_continuousLearning	2L
 *         VAL_securePortMode_autoLearn	3L
 *         VAL_securePortMode_secure	4L
 *         VAL_securePortMode_userLogin	5L
 *         VAL_securePortMode_userLoginSecure	6L
 *         VAL_securePortMode_userLoginWithOUI	7L
 *         VAL_securePortMode_macAddressWithRadius	8L
 *         VAL_securePortMode_macAddressOrUserLoginSecure	9L
 *         VAL_securePortMode_macAddressElseUserLoginSecure	10L
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_SetSecurePortMode(UI32_T lport,UI32_T secure_port_mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetSecurePortMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the learning and security modes of the port 
 * INPUT:  lport
 * OUTPUT: secure_port_mode.
 * RETURN: TRUE/FALSE.
 * NOTES:  secure_port_mode =
 *         VAL_securePortMode_noRestrictions	1L
 *         VAL_securePortMode_continuousLearning	2L
 *         VAL_securePortMode_autoLearn	3L
 *         VAL_securePortMode_secure	4L
 *         VAL_securePortMode_userLogin	5L
 *         VAL_securePortMode_userLoginSecure	6L
 *         VAL_securePortMode_userLoginWithOUI	7L
 *         VAL_securePortMode_macAddressWithRadius	8L
 *         VAL_securePortMode_macAddressOrUserLoginSecure	9L
 *         VAL_securePortMode_macAddressElseUserLoginSecure	10L
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetSecurePortMode(UI32_T lport, UI32_T *secure_port_mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_SetSecureIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Set the intrusion action to determine the action if an unauthorised device 
 *          transmits on this port.
 * INPUT:  lport,secure_intrusion_action.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  secure_intrusion_action = 
 *         VAL_secureIntrusionAction_notAvailable	1L
 *         VAL_secureIntrusionAction_noAction	2L
 *         VAL_secureIntrusionAction_disablePort	3L
 *         VAL_secureIntrusionAction_disablePortTemporarily	4L
 *         VAL_secureIntrusionAction_allowDefaultAccess	5L
 *         VAL_secureIntrusionAction_blockMacAddress	6L
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_SetSecureIntrusionAction(UI32_T lport,UI32_T secure_intrusion_action);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetSecureIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Get the intrusion action to determine the action if an unauthorised device 
 *          transmits on this port.
 * INPUT:  lport
 * OUTPUT: secure_intrusion_action.
 * RETURN: TRUE/FALSE.
 * NOTES:  secure_intrusion_action = 
 *         VAL_secureIntrusionAction_notAvailable	1L
 *         VAL_secureIntrusionAction_noAction	2L
 *         VAL_secureIntrusionAction_disablePort	3L
 *         VAL_secureIntrusionAction_disablePortTemporarily	4L
 *         VAL_secureIntrusionAction_allowDefaultAccess	5L
 *         VAL_secureIntrusionAction_blockMacAddress	6L
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetSecureIntrusionAction(UI32_T lport, UI32_T *secure_intrusion_action);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_SetSecureNumberAddresses
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
BOOL_T NETWORKACCESS_OM_SetSecureNumberAddresses(UI32_T lport,UI32_T number);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetSecureNumberAddresses
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
BOOL_T NETWORKACCESS_OM_GetSecureNumberAddresses(UI32_T lport, UI32_T *number);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetSecureNumberAddressesStored
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
BOOL_T NETWORKACCESS_OM_GetSecureNumberAddressesStored(UI32_T lport, UI32_T *secure_number_addresses_stored);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetSecureNumberAddressesAuthorized
 * ---------------------------------------------------------------------
 * PURPOSE: number of authorized addresses
 * INPUT:  lport
 * OUTPUT: addr_number
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetSecureNumberAddressesAuthorized(UI32_T lport, UI32_T *addr_number);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetSecureNumberAddressesUnauthorized
 * ---------------------------------------------------------------------
 * PURPOSE: number of unauthorized addresses
 * INPUT:  lport
 * OUTPUT: addr_number
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetSecureNumberAddressesUnauthorized(UI32_T lport, UI32_T *addr_number);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetSecureMaximumAddresses
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
BOOL_T NETWORKACCESS_OM_GetSecureMaximumAddresses(UI32_T lport, UI32_T *secure_maximum_addresses);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_GetHiddenMacNumberByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : get number of hidden mac by lport
 * INPUT    : lport
 * OUTPUT   : qty
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_GetHiddenMacNumberByPort(UI32_T lport, UI32_T *qty);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetTotalReservedSecureAddresses
 * ---------------------------------------------------------------------
 * PURPOSE: Get the total number that secureNumberAddresses had be set.
 * INPUT:  total_reserved_addresses.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetTotalReservedSecureAddresses(UI32_T *total_reserved_addresses);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetMinimumSecureAddresses
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the minimum value that secureNumberAddresses can be set to.
 * INPUT    : port_mode
 * OUTPUT   : min_addresses
 * RETURN   : TRUE -- succeeded / FALSE -- failed.
 * NOTES    : none
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetMinimumSecureAddresses(UI32_T port_mode, UI32_T *min_addresses);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetUsableSecureAddresses
 * ---------------------------------------------------------------------
 * PURPOSE: Get the maximum value that secureNumberAddresses can be set to.
 * INPUT:  lport, usable_addresses.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES: 
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetUsableSecureAddresses(UI32_T lport, UI32_T *usable_addresses);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetUnreservedSecureAddresses
 * ---------------------------------------------------------------------
 * PURPOSE: Get the number of unreserved secure addresses
 * INPUT:   none
 * OUTPUT:  unreserved_nbr
 * RETURN:  TRUE/FALSE.
 * NOTES:   none
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetUnreservedSecureAddresses(UI32_T *unreserved_nbr);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_ExtendUpperBoundByPort
 * ---------------------------------------------------------------------
 * PURPOSE: try to increase secureNumberAddresses by 1
 * INPUT:   lport
 * OUTPUT:  none
 * RETURN:  TRUE/FALSE.
 * NOTES:   if there is no unreserved secure address, return FALSE;
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_ExtendUpperBoundByPort(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_ShrinkUpperBoundByPort
 * ---------------------------------------------------------------------
 * PURPOSE: try to decrease secureNumberAddresses by 1
 * INPUT:   lport
 * OUTPUT:  none
 * RETURN:  TRUE/FALSE.
 * NOTES:   if number_addresses <= configured_number_addresses, must check port mode
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_ShrinkUpperBoundByPort(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_IsSecureAddressesFull
 * ---------------------------------------------------------------------
 * PURPOSE: check whether secure address is full or not
 * INPUT:  lport
 * OUTPUT: None.
 * RETURN: TRUE -- full / FALSE -- not yet
 * NOTES:  full ==> secureNumberAddressesStored >= secureNumberAddresses
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_IsSecureAddressesFull(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_SetSecureViolationTrapHoldoffTime
 * ---------------------------------------------------------------------
 * PURPOSE: Set the violation trap hold off time on this port.
 * INPUT:  lport, holdoff_time
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_SetSecureViolationTrapHoldoffTime(UI32_T lport, UI32_T holdoff_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetSecureViolationTrapHoldoffTime
 * ---------------------------------------------------------------------
 * PURPOSE: get the violation trap hold off time on this port.
 * INPUT:  lport
 * OUTPUT: holdoff_time
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetSecureViolationTrapHoldoffTime(UI32_T lport, UI32_T *holdoff_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_SetSecurePortSecurityControl
 * ---------------------------------------------------------------------
 * PURPOSE: Set the the system wide operation of network access control.
 * INPUT:  secure_port_security_control.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  This attribute controls the system wide operation of 
 * network access control. The configured port security options only become 
 * operational when this attribute is set to enabled.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_SetSecurePortSecurityControl(BOOL_T secure_port_security_control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetSecurePortSecurityControl
 * ---------------------------------------------------------------------
 * PURPOSE: Get the the system wide operation of network access control.
 * INPUT:  None.
 * OUTPUT: secure_port_security_control.
 * RETURN: TRUE/FALSE.
 * NOTES:  This attribute controls the system wide operation of 
 * network access control. The configured port security options only become 
 * operational when this attribute is set to enabled.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetSecurePortSecurityControl(BOOL_T *secure_port_security_control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_SetSecureRadaDefaultSessionTime
 * ---------------------------------------------------------------------
 * PURPOSE: Set the default session lifetime.
 * INPUT:  default_session_time.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  Specifies the default session lifetime in seconds before
 *         a forwarding MAC address is re-authenticated.
 *         The default time is 1800 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_SetSecureRadaDefaultSessionTime(UI32_T default_session_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetSecureRadaDefaultSessionTime
 * ---------------------------------------------------------------------
 * PURPOSE: Get the default session lifetime.
 * INPUT:  None.
 * OUTPUT: default_session_time.
 * RETURN: TRUE/FALSE.
 * NOTES:  Specifies the default session lifetime in seconds before
 *         a forwarding MAC address is re-authenticated.
 *         The default time is 1800 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetSecureRadaDefaultSessionTime(UI32_T *default_session_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_SetSecureRadaHoldoffTime
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
BOOL_T NETWORKACCESS_OM_SetSecureRadaHoldoffTime(UI32_T holdoff_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetSecureRadaHoldoffTime
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
BOOL_T NETWORKACCESS_OM_GetSecureRadaHoldoffTime(UI32_T *holdoff_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_SetSecureRadaAuthMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set the RADA authentication mode.
 * INPUT:  auth_mode.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  This controls how MAC addresses are authenticated.
 *
 * papUsernameAsMacAddress(1) -- VAL_secureRadaAuthMode_papUsernameAsMacAddress  
 *                  Authentication uses the RADIUS server by 
 *                  sending a PAP request with Username and 
 *                  Password both equal to the MAC address being
 *                  authenticated. This is the default.
 *
 * papUsernameFixed(2) -- VAL_secureRadaAuthMode_papUsernameFixed   
 *                  Authentication uses the RADIUS server by 
 *                  sending a PAP request with Username and 
 *                  Password coming from the secureRadaAuthUsername and
 *                  secureRadaAuthPassword MIB objects.  In this mode
 *                  the RADIUS server would normally take into account 
 *                  the request's calling-station-id attribute, which is  
 *                  the MAC address of the host being authenticated.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_SetSecureRadaAuthMode(UI32_T auth_mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetSecureRadaAuthMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the RADA authentication mode.
 * INPUT:  None.
 * OUTPUT: auth_mode.
 * RETURN: TRUE/FALSE.
 * NOTES:  This controls how MAC addresses are authenticated.
 *
 * papUsernameAsMacAddress(1) -- VAL_secureRadaAuthMode_papUsernameAsMacAddress 
 *                  Authentication uses the RADIUS server by 
 *                  sending a PAP request with Username and 
 *                  Password both equal to the MAC address being
 *                  authenticated. This is the default.
 *
 * papUsernameFixed(2) -- VAL_secureRadaAuthMode_papUsernameFixed   
 *                  Authentication uses the RADIUS server by 
 *                  sending a PAP request with Username and 
 *                  Password coming from the secureRadaAuthUsername and
 *                  secureRadaAuthPassword MIB objects.  In this mode
 *                  the RADIUS server would normally take into account 
 *                  the request's calling-station-id attribute, which is  
 *                  the MAC address of the host being authenticated.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetSecureRadaAuthMode(UI32_T *auth_mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_SetSecureRadaAuthUsername
 * ---------------------------------------------------------------------
 * PURPOSE: Set the RADA authenticate username
 * INPUT:  auth_username.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  This is the username used for authentication requests
 *         where secureRadaAuthMode is papUsernameFixed.
 *         Default shall be 'mac'.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_SetSecureRadaAuthUsername(UI8_T *auth_username);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetSecureRadaAuthUsername
 * ---------------------------------------------------------------------
 * PURPOSE: Get the RADA authenticate username.
 * INPUT:  None.
 * OUTPUT: auth_username.
 * RETURN: TRUE/FALSE.
 * NOTES:  This is the username used for authentication requests
 *         where secureRadaAuthMode is papUsernameFixed.
 *         Default shall be 'mac'.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetSecureRadaAuthUsername(UI8_T *auth_username);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_SetSecureRadaAuthPassword
 * ---------------------------------------------------------------------
 * PURPOSE: Set the RADA authenticate password.
 * INPUT:  auth_password.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  This is the password used for authentication requests
 *         where secureRadaAuthMode is papUsernameFixed.
 *         Default shall be a null string.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_SetSecureRadaAuthPassword(UI8_T *auth_password);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetSecureRadaAuthPassword
 * ---------------------------------------------------------------------
 * PURPOSE: Get the RADA authenticate password.
 * INPUT:  None.
 * OUTPUT: auth_password.
 * RETURN: TRUE/FALSE.
 * NOTES:  This is the password used for authentication requests
 *         where secureRadaAuthMode is papUsernameFixed.
 *         Default shall be a null string.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetSecureRadaAuthPassword(UI8_T *auth_password);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_SetPaePortAssignEnable
 * ---------------------------------------------------------------------
 * PURPOSE: Set 3Com PAE port assignment configuration control 
 * INPUT:  lport,
 *         assign_enable = 
 *          VAL_a3ComPaePortAssignEnable_true	  1L
 *          VAL_a3ComPaePortAssignEnable_false	2L 
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES: The user-based port configuration control. Setting this attribute
 *        TRUE causes the port to be configured with any configuration 
 *        parameters supplied by the authentication server. Setting this
 *        attribute to FALSE causes any configuration parameters supplied
 *        by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_SetPaePortAssignEnable(UI32_T lport,UI32_T assign_enable);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetPaePortAssignEnable
 * ---------------------------------------------------------------------
 * PURPOSE: Get 3Com PAE port assignment configuration control 
 * INPUT:  lport.
 * OUTPUT: assign_enable = 
 *          VAL_a3ComPaePortAssignEnable_true	  1L
 *          VAL_a3ComPaePortAssignEnable_false	2L 
 * RETURN: TRUE/FALSE.
 * NOTES: The user-based port configuration control. Setting this attribute
 *        TRUE causes the port to be configured with any configuration 
 *        parameters supplied by the authentication server. Setting this
 *        attribute to FALSE causes any configuration parameters supplied
 *        by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetPaePortAssignEnable(UI32_T lport,UI32_T *assign_enable);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_GetSecurePortEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will copy secure port entry by the lport.
 * INPUT    : lport
 * OUTPUT   : entry : The secure port entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetSecurePortEntry(UI32_T lport, NETWORKACCESS_SecurePortTable_T *entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_CalculateSecureUnitAndPort
 *-----------------------------------------------------------------------------------
 * PURPOSE  : initialize the unit & port of secure port when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port inserted
 *            number_of_port        -- the number of ports on the inserted module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_CalculateSecureUnitAndPort(UI32_T starting_port_ifindex, UI32_T number_of_port);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_SetSecurePortIsBlocked
 * ---------------------------------------------------------------------
 * PURPOSE: set is_block flag of secure port
 * INPUT:  lport, is_blocked
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_SetSecurePortIsBlocked(UI32_T lport, BOOL_T is_blocked);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetSecurePortIsBlocked
 * ---------------------------------------------------------------------
 * PURPOSE: get is_block flag of secure port
 * INPUT:  lport.
 * OUTPUT: is_blocked
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetSecurePortIsBlocked(UI32_T lport, BOOL_T *is_blocked);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_SetSecurePortVlanAssignmentResult
 * ---------------------------------------------------------------------
 * PURPOSE: set vlan_profile of secure port
 * INPUT:  lport, assignment_result
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_SetSecurePortVlanAssignmentResult(UI32_T lport, const UI8_T *assignment_result);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_GetSecurePortVlanAssignmentResult
 * ---------------------------------------------------------------------
 * PURPOSE: gt vlan_profile of secure port
 * INPUT:  lport, buffer_size
 * OUTPUT: assignment_result
 * RETURN: TRUE/FALSE.
 * NOTES:  strlen(profile) can't large than buffer_size - 1
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetSecurePortVlanAssignmentResult(UI32_T lport, UI8_T *assignment_result, UI32_T buffer_size);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_SetSecurePortQoSAssignmentResult
 * ---------------------------------------------------------------------
 * PURPOSE: set qos_profile of secure port
 * INPUT:  lport, assignment_result
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_SetSecurePortQoSAssignmentResult(UI32_T lport, const UI8_T *assignment_result);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_CreateSecureAddressEntry
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
BOOL_T NETWORKACCESS_OM_CreateSecureAddressEntry(const NETWORKACCESS_SecureMacTable_T *mac_entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_DoesPortAllowCreateSecureAddrEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : whether the specified port allow to create mac or not
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE -- yes / FALSE -- no
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_DoesPortAllowCreateSecureAddrEntry(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_UpdateSecureAddressEntryByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: update mac entry by mac_index (om & hisam)
 * INPUT:  mac_entry
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  if not existed, return FALSE
 *         addr_row_status must be active or notInService
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_UpdateSecureAddressEntryByIndex(const NETWORKACCESS_SecureMacTable_T *mac_entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_ReplaceOldestSecureAddressEntryByPort
 * ---------------------------------------------------------------------
 * PURPOSE: replace mac entry by solt + port (om & hisam)
 * INPUT:  mac_entry (new mac entry)
 * OUTPUT: mac_entry (the replaced mac entry, mac_index should be ignored)
 * RETURN: TRUE/FALSE.
 * NOTES:  mac_index will be ingored
 *         addr_row_status must be active or notInService
 *         replace unauthorized mac first. if unauthorized mac not found then replace authorized mac
 *         if can't find any entry to replace, return FALSE
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_ReplaceOldestSecureAddressEntryByPort(NETWORKACCESS_SecureMacTable_T *mac_entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_GetSecureAddressEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get secure address entry entry by the unit, port and mac.
 * INPUT    : entry->slot_index, entry->port_index, entry->secure_mac
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetSecureAddressEntry(NETWORKACCESS_SecureMacTable_T *entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_GetSecureAddressEntryByIndex
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get secure address entry entry by the unit, port and mac.
 * INPUT    : entry->mac_index
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetSecureAddressEntryByIndex(NETWORKACCESS_SecureMacTable_T *entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_GetSecureAddressEntryByMacFlag
 *-----------------------------------------------------------------------------------
 * PURPOSE  : get secure address entry entry by unt, port and mac_flag
 * INPUT    : entry->slot_index, entry->port_index, entry->mac_flag
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : return the first entry satisfied (entry->mac_flag & om_entry->mac_flag)
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetSecureAddressEntryByMacFlag(NETWORKACCESS_SecureMacTable_T *entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_GetNextSecureAddressEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get next secure address entry by the unit, port and the mac_address.
 * INPUT    : entry->slot_index, entry->port_index, entry->secure_mac
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetNextSecureAddressEntry(NETWORKACCESS_SecureMacTable_T *entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_GetNextRunningSecureAddressEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : get next running secure address entry by the unit, port and the mac_address.
 * INPUT    : entry->slot_index, entry->port_index, entry->secure_mac
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_GetNextRunningSecureAddressEntry(NETWORKACCESS_SecureMacTable_T *entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_IsSecureAddressAdminConfigured
 *-----------------------------------------------------------------------------------
 * PURPOSE  : check whether the secure address is administrative configured
 * INPUT    : mac_index
 * OUTPUT   : none
 * RETURN   : TRUE -- admin configured / FALSE -- not admin configured
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_IsSecureAddressAdminConfigured(UI32_T mac_index);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_SetSecureAddrRowStatus
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will set secure port entry status by the unit, port and the mac_address.
 * INPUT    : entry->slot_index, entry->port_index, entry->secure_mac, entry->addr_row_status
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : addr_row_status must be active or notInService
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_SetSecureAddrRowStatus(const NETWORKACCESS_SecureMacTable_T *entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_SetSecureAddrAppliedToChipByIndex
 *-----------------------------------------------------------------------------------
 * PURPOSE  : set secure port entry applied_to_chip by mac_index
 * INPUT    : mac_index, applied_to_chip
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_SetSecureAddrAppliedToChipByIndex(UI32_T mac_index, BOOL_T applied_to_chip);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_SetSecureAddrWriteToAmtrByIndex
 *-----------------------------------------------------------------------------------
 * PURPOSE  : set secure port entry write_to_amtr by mac_index
 * INPUT    : mac_index, write_to_amtr
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_SetSecureAddrWriteToAmtrByIndex(UI32_T mac_index, BOOL_T write_to_amtr);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_DeleteSecureAddressEntryByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: delete mac entry by mac_index (om & hisam)
 * INPUT:  mac_index
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  if not existed, return FALSE
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_DeleteSecureAddressEntryByIndex(UI32_T mac_index);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_DeleteSecureAddressEntryBySecureKey
 * ---------------------------------------------------------------------
 * PURPOSE: delete mac entry by key (om & hisam)
 * INPUT:  key
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  if not existed, return FALSE
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_DeleteSecureAddressEntryBySecureKey(const NETWORKACCESS_OM_SecureKey_T *key);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_DeleteAllSecureAddress
 * ---------------------------------------------------------------------
 * PURPOSE: delete all (authorized & unauthorized) mac addresses from om & hisam
 * INPUT:  none
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_DeleteAllSecureAddress();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_DeleteAllLearnedSecureAddress
 * ---------------------------------------------------------------------
 * PURPOSE: delete all learned mac from om & hisam except manually configured
 * INPUT:  none
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_DeleteAllLearnedSecureAddress();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_DeleteAllSecureAddressByPort
 * ---------------------------------------------------------------------
 * PURPOSE: delete all learned (authorized & unauthorized) mac addresses from om & hisam
 * INPUT:  lport
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_DeleteAllSecureAddressByPort(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_DeleteAllLearnedSecureAddressByPort
 * ---------------------------------------------------------------------
 * PURPOSE: delete all mac addresses from om & hisam except manually configured mac
 * INPUT:  lport
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_DeleteAllLearnedSecureAddressByPort(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_OM_DeleteHiddenMacByPort
 * ---------------------------------------------------------------------
 * PURPOSE: delete all hidden mac addresses from om & hisam by lport
 * INPUT:  lport
 * OUTPUT: none
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_OM_DeleteHiddenMacByPort(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_GetHisamRecordBySecureKey
 *-------------------------------------------------------------------------
 * PURPOSE  : get entry by NETWORKACCESS_OM_SecureKey_T
 * INPUT    : key
 * OUTPUT   : hisam_entry
 * RETURN   : TRUE -- succeeded, FALSE -- failed
 * NOTE     : None
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_GetHisamRecordBySecureKey(const NETWORKACCESS_OM_SecureKey_T *key, NETWORKACCESS_OM_HISAMentry_T *hisam_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_GetNextHisamRecordBySecureKey
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next record
 * INPUT    : key
 * OUTPUT   : key, hisam_entry
 * RETURN   : TRUE -- succeeded, FALSE -- failed
 * NOTE     : None
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_GetNextHisamRecordBySecureKey(NETWORKACCESS_OM_SecureKey_T *key, NETWORKACCESS_OM_HISAMentry_T *hisam_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_DoesRecordExistInHisamBySecureKey
 *-------------------------------------------------------------------------
 * PURPOSE  : check whether the specified record exist or not
 * INPUT    : key
 * OUTPUT   : none
 * RETURN   : TRUE - exist, FALSE - not exist
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_DoesRecordExistInHisamBySecureKey(const NETWORKACCESS_OM_SecureKey_T *key);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_GetHisamRecordByOldestKey
 *-------------------------------------------------------------------------
 * PURPOSE  : get entry by NETWORKACCESS_OM_OldestKey_T
 * INPUT    : key
 * OUTPUT   : hisam_entry
 * RETURN   : TRUE -- succeeded, FALSE -- failed
 * NOTE     : None
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_GetHisamRecordByOldestKey(const NETWORKACCESS_OM_OldestKey_T *key, NETWORKACCESS_OM_HISAMentry_T *hisam_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_GetNextHisamRecordByOldestKey
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next record
 * INPUT    : key
 * OUTPUT   : key, hisam_entry
 * RETURN   : TRUE -- succeeded, FALSE -- failed
 * NOTE     : None
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_GetNextHisamRecordByOldestKey(NETWORKACCESS_OM_OldestKey_T *key, NETWORKACCESS_OM_HISAMentry_T *hisam_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_GetHisamRecordByExpireKey
 *-------------------------------------------------------------------------
 * PURPOSE  : get entry by NETWORKACCESS_OM_ExpireKey_T
 * INPUT    : key
 * OUTPUT   : hisam_entry
 * RETURN   : TRUE -- succeeded, FALSE -- failed
 * NOTE     : None
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_GetHisamRecordByExpireKey(const NETWORKACCESS_OM_ExpireKey_T *key, NETWORKACCESS_OM_HISAMentry_T *hisam_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_GetNextHisamRecordByExpireKey
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next record
 * INPUT    : key
 * OUTPUT   : key, hisam_entry
 * RETURN   : TRUE -- succeeded, FALSE -- failed
 * NOTE     : None
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_GetNextHisamRecordByExpireKey(NETWORKACCESS_OM_ExpireKey_T *key, NETWORKACCESS_OM_HISAMentry_T *hisam_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_GetNextHisamRecordByMacKey
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next record
 * INPUT    : key
 * OUTPUT   : key, hisam_entry
 * RETURN   : TRUE -- succeeded, FALSE -- failed
 * NOTE     : None
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_GetNextHisamRecordByMacKey(NETWORKACCESS_OM_MacAdrKey_T *key, NETWORKACCESS_OM_HISAMentry_T *hisam_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_SetPortStateMachine
 *-------------------------------------------------------------------------
 * PURPOSE  : set the specific port's state machine (state_machine->lport)
 * INPUT    : state_machine
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_SetPortStateMachine(const NETWORKACCESS_OM_StateMachine_T *state_machine);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_GetPortStateMachine
 *-------------------------------------------------------------------------
 * PURPOSE  : get the specific port's state machine
 * INPUT    : lport
 * OUTPUT   : state_machine
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_GetPortStateMachine(UI32_T lport, NETWORKACCESS_OM_StateMachine_T *state_machine);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_GetPortStateMachineRunningPortMode
 *-------------------------------------------------------------------------
 * PURPOSE  : get the specific port's running_port_mode
 * INPUT    : lport
 * OUTPUT   : running_port_mode
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_GetPortStateMachineRunningPortMode(UI32_T lport, UI32_T *running_port_mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_ClearStateMachineNewMacMsg
 *-------------------------------------------------------------------------
 * PURPOSE  : clear the specific port's new_mac_msg of state machine
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_ClearStateMachineNewMacMsg(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_ClearStateMachineRadiusMsg
 *-------------------------------------------------------------------------
 * PURPOSE  : clear the specific port's radius_msg of state machine
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_ClearStateMachineRadiusMsg(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_ClearStateMachineDot1xMsg
 *-------------------------------------------------------------------------
 * PURPOSE  : clear the specific port's dot1x_msg of state machine
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_ClearStateMachineDot1xMsg(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_ClearStateMachineDot1xLogonFlag
 *-------------------------------------------------------------------------
 * PURPOSE  : clear the specific port's dot1x_logon flag of state machine
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_ClearStateMachineDot1xLogonFlag(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_StopStateMachineDoAuthentication
 *-------------------------------------------------------------------------
 * PURPOSE  : state become to idle and turn off is_authenticating flag
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_StopStateMachineDoAuthentication(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_IsStateMachineAuthenticating
 *-------------------------------------------------------------------------
 * PURPOSE  : check whether the specific port's is_authenticating flag of PSEC state machine
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE - yes, FALSE - no
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_IsStateMachineAuthenticating(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_IsThisMacAuthenticatingMac
 *-------------------------------------------------------------------------
 * PURPOSE  : check whether the specific mac on a port is authenticating mac or not
 * INPUT    : lport, mac
 * OUTPUT   : none
 * RETURN   : TRUE - yes, FALSE - no
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_IsThisMacAuthenticatingMac(UI32_T lport, const UI8_T *mac);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_IsThisMacAuthorizedByDot1x
 *-------------------------------------------------------------------------
 * PURPOSE  : check whether the specified (lport, mac) is authorized by dot1x
 * INPUT    : lport, mac
 * OUTPUT   : none
 * RETURN   : TRUE - yes, FALSE - no
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_IsThisMacAuthorizedByDot1x(UI32_T lport, const UI8_T *mac);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_IsThisMacLearnedFromEapPacket
 *-------------------------------------------------------------------------
 * PURPOSE  : check whether the specific mac on a port is learned from an EAP packet or not
 * INPUT    : lport, mac
 * OUTPUT   : none
 * RETURN   : TRUE - yes, FALSE - no
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_IsThisMacLearnedFromEapPacket(UI32_T lport, const UI8_T *mac);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_IsThisMacUnauthorizedMac
 *-------------------------------------------------------------------------
 * PURPOSE  : check whether the specific mac on a port is unauthorized mac or not
 * INPUT    : lport, mac
 * OUTPUT   : none
 * RETURN   : TRUE - yes, FALSE - no
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_IsThisMacUnauthorizedMac(UI32_T lport, const UI8_T *mac);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_CreatePortDisableTimer
 *-------------------------------------------------------------------------
 * PURPOSE  : create disablePortTemporarily timer
 * INPUT    : lport, expire_time
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_CreatePortDisableTimer(UI32_T lport, UI32_T expire_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_DestroyFirstPortDisableTimer
 *-------------------------------------------------------------------------
 * PURPOSE  : destroy disablePortTemporarily timer
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETWORKACCESS_OM_DestroyFirstPortDisableTimer();

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_GetFirstPortDisableTimer
 *-------------------------------------------------------------------------
 * PURPOSE  : get first disablePortTemporarily timer
 * INPUT    : none
 * OUTPUT   : lport, expire_time
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_GetFirstPortDisableTimer(UI32_T *lport, UI32_T *expire_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_DuplicatePortEapData
 *-------------------------------------------------------------------------
 * PURPOSE  : duplicate eap data in om by lport
 * INPUT    : lport, eap_data
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_DuplicatePortEapData(UI32_T lport, const NETWORKACCESS_EAP_DATA_T *eap_data);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_DestroyFirstPortEapData
 *-------------------------------------------------------------------------
 * PURPOSE  : destroy the first eap data entry by lport
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETWORKACCESS_OM_DestroyFirstPortEapPacket(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_DestroyAllPortEapData
 *-------------------------------------------------------------------------
 * PURPOSE  : destroy all eap data entry by lport
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETWORKACCESS_OM_DestroyAllPortEapData(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_GetFirstPortEapData
 *-------------------------------------------------------------------------
 * PURPOSE  : get the first eap data by lport
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : if succeeded, return a eap data; NULL - failed
 * NOTES    : MUST call NETWORKACCESS_OM_FreeEapData() to free memory
 *-------------------------------------------------------------------------*/
NETWORKACCESS_EAP_DATA_T *NETWORKACCESS_OM_GetFirstPortEapData(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_FreeEapData
 *-------------------------------------------------------------------------
 * PURPOSE  : free eap data
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : none
 * NOTE     : none
 *-------------------------------------------------------------------------*/
void NETWORKACCESS_OM_FreeEapData(NETWORKACCESS_EAP_DATA_T **eap_data);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_GetVlanListByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : get vlan list by lport
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : if succeeded, return a vlan list; NULL - failed
 * NOTE     : MUST call NETWORKACCESS_OM_FreeVlanList() to free memory
 *-------------------------------------------------------------------------*/
NETWORKACCESS_OM_VlanList_T* NETWORKACCESS_OM_GetVlanListByPort(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_FreeVlanList
 *-------------------------------------------------------------------------
 * PURPOSE  : free vlan list
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : none
 * NOTE     : none
 *-------------------------------------------------------------------------*/
void NETWORKACCESS_OM_FreeVlanList(NETWORKACCESS_OM_VlanList_T **vlan_list);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_ExcludeVlanListAFromB
 *-------------------------------------------------------------------------
 * PURPOSE  : exclude the vlans which existed in A from B
 * INPUT    : vlist_a, vlist_b
 * OUTPUT   : vlist_b
 * RETURN   : if succeeded, return a vlan list; NULL - failed
 * NOTE     : e.g. A(1, 2, 3) B(2, 3, 4) ==> (4)
 *-------------------------------------------------------------------------*/
NETWORKACCESS_OM_VlanList_T* NETWORKACCESS_OM_ExcludeVlanListAFromB(const NETWORKACCESS_OM_VlanList_T *vlist_a, const NETWORKACCESS_OM_VlanList_T *vlist_b);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_ExcludeVlanListBWhichNotInA
 *-------------------------------------------------------------------------
 * PURPOSE  : exclude the vlans which doesn't exist in A from B
 * INPUT    : vlist_a, vlist_b
 * OUTPUT   : vlist_b
 * RETURN   : if succeeded, return a vlan list; NULL - failed
 * NOTE     : e.g. A(1, 2, 3) B(2, 3, 4) ==> (2, 3)
 *-------------------------------------------------------------------------*/
NETWORKACCESS_OM_VlanList_T* NETWORKACCESS_OM_ExcludeVlanListBWhichNotInA(const NETWORKACCESS_OM_VlanList_T *vlist_a, const NETWORKACCESS_OM_VlanList_T *vlist_b);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_OM_CompareVlanList
 *-------------------------------------------------------------------------
 * PURPOSE  : compare two vlan list
 * INPUT    : vlist_a, vlist_b
 * OUTPUT   : none
 * RETURN   : return 1 if a > b, return 0 if a == b, return -1 if a < b
 * NOTE     : none
 *-------------------------------------------------------------------------*/
I32_T NETWORKACCESS_OM_CompareVlanList(const NETWORKACCESS_OM_VlanList_T *vlist_a, const NETWORKACCESS_OM_VlanList_T *vlist_b);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_ResetStatisticData
 *-------------------------------------------------------------------------
 * PURPOSE  : reset all counters of statistic data to 0
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETWORKACCESS_OM_ResetStatisticData();

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_IncNewMacCounter
 *-------------------------------------------------------------------------
 * PURPOSE  : increase new mac callback counter by 1
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETWORKACCESS_OM_IncNewMacCounter();

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_IncEapCounter
 *-------------------------------------------------------------------------
 * PURPOSE  : increase eap callback counter by 1
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETWORKACCESS_OM_IncEapCounter();

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_IncRadiusResultCounter
 *-------------------------------------------------------------------------
 * PURPOSE  : increase radius result cookie counter by 1
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETWORKACCESS_OM_IncRadiusResultCounter();

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_IncDot1xResultCounter
 *-------------------------------------------------------------------------
 * PURPOSE  : increase dot1x result cookie counter by 1
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETWORKACCESS_OM_IncDot1xResultCounter();

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_IncRadiusAuthCounter
 *-------------------------------------------------------------------------
 * PURPOSE  : increase radius authentication counter by 1
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETWORKACCESS_OM_IncRadiusAuthCounter();

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_IncDot1xAuthCounter
 *-------------------------------------------------------------------------
 * PURPOSE  : increase dot1x authentication counter by 1
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETWORKACCESS_OM_IncDot1xAuthCounter();

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_IncAmtrSetPsecAddrCounter
 *-------------------------------------------------------------------------
 * PURPOSE  : increase amtr set psec address counter by 1
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETWORKACCESS_OM_IncAmtrSetPsecAddrCounter();

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_IncAmtrDeleteAddrCounter
 *-------------------------------------------------------------------------
 * PURPOSE  : increase amtr delete address counter by 1
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void NETWORKACCESS_OM_IncAmtrDeleteAddrCounter();

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_GetStatisticData
 *-------------------------------------------------------------------------
 * PURPOSE  : get statistic data
 * INPUT    : none
 * OUTPUT   : data
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_GetStatisticData(NETWORKACCESS_OM_StatisticData_T *data);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_SetDebugFlag
 *-------------------------------------------------------------------------
 * PURPOSE  : set backdoor debug flag
 * INPUT    : debug_flag
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_SetDebugFlag(UI32_T debug_flag);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_GetDebugFlag
 *-------------------------------------------------------------------------
 * PURPOSE  : get backdoor debug flag
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : debug flag
 * NOTES    : none
 *-------------------------------------------------------------------------*/
UI32_T NETWORKACCESS_OM_GetDebugFlag();

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_SetProvisionCompleteFlag
 *-------------------------------------------------------------------------
 * PURPOSE  : set provision complete flag
 * INPUT    : provision_complete_flag
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_SetProvisionCompleteFlag(BOOL_T provision_complete_flag);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_OM_GetProvisionCompleteFlag
 *-------------------------------------------------------------------------
 * PURPOSE  : get provision complete flag
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : networkaccess_provision_complete_flag
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_OM_GetProvisionCompleteFlag();

#endif /*NETWORKACCESS_OM_H*/