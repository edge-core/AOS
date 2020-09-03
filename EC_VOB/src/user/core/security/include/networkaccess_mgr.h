/* Project Name: New Feature
 * File_Name : networkaccess_mgr.h
 * Purpose     : NETWORKACCESS initiation and NETWORKACCESS task creation
 *
 * 2004/09/29    : Kevin Cheng  Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (ACP_V3)
 */
 
#ifndef	NETWORKACCESS_MGR_H
#define	NETWORKACCESS_MGR_H

/* INCLUDE FILE DECLARATIONS */
#include "sys_type.h" 
#include "sys_adpt.h"
#include "security_backdoor.h"
#include "networkaccess_type.h"
#include "1x_types.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define NETWORKACCESS_SUPPORT_ACCTON_BACKDOOR      (FALSE && SECURITY_SUPPORT_ACCTON_BACKDOOR) /* support backdoor functions */

/* MACRO FUNCTION DECLARATIONS */

/* DATA TYPE DECLARATIONS
 */
typedef enum NETWORKACCESS_FunctionFailure_E
{
    NETWORKACCESS_FUNCTION_SUCCEEDED = 0,

    NETWORKACCESS_COZ_NOT_NORMAL_PORT,                  /* port type != SWCTRL_LPORT_NORMAL_PORT */
    NETWORKACCESS_COZ_INVLID_MAC_ADDRESS,               /* e.g. broadcast, multicast mac */
    NETWORKACCESS_COZ_IS_TRUNK_PORT,
    NETWORKACCESS_COZ_LACP_ENABLED,
    NETWORKACCESS_COZ_NOT_ALLOW_CREATE_MAC_ADDRESS,     /* don't allow create mac in current port mode */
    NETWORKACCESS_COZ_MAC_ALREADY_EXIST,                /* mac existed, please check the lport parameter */
    NETWORKACCESS_COZ_NO_MORE_SPACE_TO_CREATE,          /* system capacity is full */

    NETWORKACCESS_COZ_INTERNAL_ERROR,                   /* program internal error */
    NETWORKACCESS_COZ_UNKNOWN_REASON,
} NETWORKACCESS_FunctionFailure_T;

/* follow 802.1X 9.4.4.1.3 Outputs (g) */
typedef enum NETWORKACCESS_Authentic_E
{
    NETWORKACCESS_AUTH_BY_UNKNOWN                   = 1, /* equal 1 because cgconfig_network_access_detail_htm.c */
    NETWORKACCESS_AUTH_BY_REMOTE,                       /* remote */
    NETWORKACCESS_AUTH_BY_LOCAL,                        /* local */
} NETWORKACCESS_Authentic_T;

/* follow 802.1X 9.4.4.1.3 Outputs (i) */
typedef enum NETWORKACCESS_TerminateCause_E
{
    NETWORKACCESS_TERM_BY_UNKNOWN,

    NETWORKACCESS_TERM_BY_SUPPLICANT_LOGOFF,            /* supplicant logoff */
    NETWORKACCESS_TERM_BY_PORT_FAILURE,                 /* port failure */
    NETWORKACCESS_TERM_BY_SUPPLICANT_RESTART,           /* supplicant restart */
    NETWORKACCESS_TERM_BY_REAUTH_FAILED,                /* reauthentication failed */
    NETWORKACCESS_TERM_BY_AUTH_CONTROL_FORCE_UNAUTH,    /* force unauthorized */
    NETWORKACCESS_TERM_BY_PORT_REINIT,                  /* reinitialized */
    NETWORKACCESS_TERM_BY_PORT_ADMIN_DISABLED,          /* admin disabled */
    NETWORKACCESS_TERM_BY_NOT_TERMINATED_YET,           /* not terminated yet */
} NETWORKACCESS_TerminateCause_T;

typedef struct NETWORKACCESS_SecurePortEntry_S
{
    UI32_T  slot_index;
    UI32_T  port_index;
    UI32_T  port_mode;
    UI32_T  need_to_know_mode;
    UI32_T  intrusion_action;
    UI32_T  number_addresses;
    UI32_T  number_addresses_stored;
    UI32_T  maximum_addresses;
    UI32_T  nbr_of_authorized_addresses;
    /* unauthorized mac = number_addresses_stored - nbr_of_authorized_addresses */
} NETWORKACCESS_SecurePortEntry_T;

typedef struct NETWORKACCESS_SecureAddressEntry_S
{
    UI32_T  addr_slot_index;
    UI32_T  addr_port_index;
    UI8_T   addr_MAC[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T  addr_row_status; 	
} NETWORKACCESS_SecureAddressEntry_T;

typedef struct NETWORKACCESS_SecureOUIEntry_S
{
    UI32_T  secureOUISlotIndex;
    UI8_T   secureOUI[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T  secureOUIRowStatus;
} NETWORKACCESS_SecureOUIEntry_T;

typedef struct NETWORKACCESS_PaePortEntry_S
{
	UI32_T  dot1xPaePortNumber;
	UI32_T  pae_port_assign_enable;
	UI8_T   pae_port_vlan_assignment[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST+1];
	UI8_T   pae_port_qos_assignment[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE+1];
} NETWORKACCESS_PaePortEntry_T;

typedef struct NETWORKACCESS_SecureRadaGroup_S
{
	UI32_T  secure_rada_default_session_time;
	UI32_T  secure_rada_holdoff_time;
	UI32_T  secure_rada_auth_mode;
	UI8_T   secureRadaAuthUsername[MAXSIZE_secureRadaAuthUsername+1];
	UI8_T   secureRadaAuthPassword[MAXSIZE_secureRadaAuthPassword+1];
} NETWORKACCESS_SecureRadaGroup_T;

typedef struct NETWORKACCESS_PortDetailData_S
{
    NETWORKACCESS_Authentic_T       auth_method;
    NETWORKACCESS_TerminateCause_T  termination_cause;

    UI8_T   user_name[DOT1X_USERNAME_LENGTH + 1];
    UI32_T  session_time;
    UI32_T  rx_octets;
    UI32_T  tx_octets;
    UI32_T  rx_frames;
    UI32_T  tx_frames;
    BOOL_T  is_ever_auth_success;
} NETWORKACCESS_PortDetailData_T;

typedef struct NETWORKACCESS_FailureReason_S
{
    NETWORKACCESS_FunctionFailure_T reason;
    UI32_T                          lport;      /* reason caused by which port */
} NETWORKACCESS_FailureReason_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
/*---------------------------------------------------------------------------
 * Routine Name : NETWORKACCESS_MGR_InitiateSystemResources                                          
 *---------------------------------------------------------------------------
 * Function : Initialize NETWORKACCESS's MGR .	                                     
 * Input    : None                                                           
 * Output   : None                                                               
 * Return   : TRUE/FALSE                                                
 * Note     : None                                                               
 *---------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_InitiateSystemResources(void);

/*---------------------------------------------------------------------------
 * Routine Name : NETWORKACCESS_MGR_Create_InterCSC_Relation
 *---------------------------------------------------------------------------
 * Function : This function initializes all function pointer registration operations.
 * Input    : None
 * Output   : None
 * Return   : None
 * Note     : None
 *---------------------------------------------------------------------------
 */
void NETWORKACCESS_MGR_Create_InterCSC_Relation(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - NETWORKACCESS_MGR_EnterMasterMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the NETWORKACCESS enter the master mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void NETWORKACCESS_MGR_EnterMasterMode();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - NETWORKACCESS_MGR_EnterSlaveMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the NETWORKACCESS enter the Slave mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void NETWORKACCESS_MGR_EnterSlaveMode();

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - NETWORKACCESS_MGR_SetTransitionMode
 *---------------------------------------------------------------------------
 * PURPOSE:  Set transition mode
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void NETWORKACCESS_MGR_SetTransitionMode(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - NETWORKACCESS_MGR_EnterTransitionMode Mode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the NETWORKACCESS enter the transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void NETWORKACCESS_MGR_EnterTransitionMode();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - NETWORKACCESS_MGR_GetCurrentOperationMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function return the current opearion mode of networkaccess's task
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   operation_mode
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T NETWORKACCESS_MGR_GetCurrentOperationMode();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_ProcessNewMacMsg
 * ---------------------------------------------------------------------
 * PURPOSE: Process the data that dequeue from new mac message queue 
 * INPUT:  new_mac_msg_ptr.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_ProcessNewMacMsg(NETWORKACCESS_NEWMAC_MSGQ_T *new_mac_msg_ptr);
 
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_ProcessTimerEvent
 * ---------------------------------------------------------------------
 * PURPOSE: Process timer alarm event from timer
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_ProcessTimerEvent();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_ProcessRadiusMsg
 * ---------------------------------------------------------------------
 * PURPOSE: Process the data that dequeue from RADIUS message queue 
 * INPUT:  radius_msg_ptr
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_ProcessRadiusMsg(NETWORKACCESS_RADIUS_MSGQ_T *radius_msg_ptr);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_ProcessDot1xMsg
 * ---------------------------------------------------------------------
 * PURPOSE: Process the data that dequeue from dot1x message queue 
 * INPUT:  radius_msg_ptr
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_ProcessDot1xMsg(NETWORKACCESS_DOT1X_MSGQ_T *dot1x_msg_ptr);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_ProcessLinkStateChangeMsg
 * ---------------------------------------------------------------------
 * PURPOSE: Process the data that dequeue from link state change message queue 
 * INPUT:  msg
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_ProcessLinkStateChangeMsg(NETWORKACCESS_LinkStateChange_MSGQ_T *msg);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_ProcessVlanModifiedMsg
 * ---------------------------------------------------------------------
 * PURPOSE: Process the data that dequeue from vlan modified message queue 
 * INPUT:  msg
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_ProcessVlanModifiedMsg(NETWORKACCESS_VlanModified_MSGQ_T *msg);

/*************************
 * For UI (CLI/WEB/SNMP) *
 *************************/
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_SetSecurePortMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set the learning and security modes of the port 
 * INPUT:  unit,port,secure_port_mode.
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
BOOL_T NETWORKACCESS_MGR_SetSecurePortMode(UI32_T unit,UI32_T port,UI32_T secure_port_mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetSecurePortMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the learning and security modes of the port 
 * INPUT:  unit,port
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
BOOL_T NETWORKACCESS_MGR_GetSecurePortMode(UI32_T unit,UI32_T port, UI32_T *secure_port_mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetRunningSecurePortMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default secure_port_mode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   unit,port.
 * OUTPUT:  secure_port_mode
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETWORKACCESS_MGR_GetRunningSecurePortMode(UI32_T unit,UI32_T port,UI32_T *secure_port_mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_SetSecureIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Set the intrusion action to determine the action if an unauthorised device 
 *          transmits on this port.
 * INPUT:  unit,port,secure_intrusion_action.
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
BOOL_T NETWORKACCESS_MGR_SetSecureIntrusionAction(UI32_T unit,UI32_T port,UI32_T secure_intrusion_action);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetSecureIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Get the intrusion action to determine the action if an unauthorised device 
 *          transmits on this port.
 * INPUT:  unit,port
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
BOOL_T NETWORKACCESS_MGR_GetSecureIntrusionAction(UI32_T unit,UI32_T port, UI32_T *secure_intrusion_action);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetRunningSecureIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default secure_intrusion_action is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   unit,port.
 * OUTPUT:  secure_intrusion_action
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETWORKACCESS_MGR_GetRunningSecureIntrusionAction(UI32_T unit,UI32_T port,UI32_T *secure_intrusion_action);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_VerifySecureIntrusionAction
 *-----------------------------------------------------------------------------------
 * PURPOSE  : whether (port_mode, intrusion_action) is valid pair or not
 * INPUT    : port_mode, intrusion_action
 * OUTPUT   : intrusion_action, is_valid
 * RETURN   : TRUE -- succeeded, FALSE -- failed
 * NOTES    : if (is_valid == FALSE), intrusion_action will be set to default action
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_VerifySecureIntrusionAction(UI32_T port_mode, UI32_T *intrusion_action, BOOL_T *is_valid);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_SetSecureNumberAddresses
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will set secureNumberAddresses by the unit and the port.
 * INPUT    : unit : secureSlotIndex.
 *            port : securePortIndex.
 *            number:secureNumberAddresses
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
BOOL_T NETWORKACCESS_MGR_SetSecureNumberAddresses(UI32_T unit,UI32_T port,UI32_T number);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetSecureNumberAddresses
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
BOOL_T NETWORKACCESS_MGR_GetSecureNumberAddresses(UI32_T unit,UI32_T port, UI32_T *number);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetRunningSecureNumberAddresses
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default number is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   unit,port.
 * OUTPUT:  number:secureNumberAddresses
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETWORKACCESS_MGR_GetRunningSecureNumberAddresses(UI32_T unit,UI32_T port,UI32_T *number);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetSecureNumberAddressesStored
 * ---------------------------------------------------------------------
 * PURPOSE: The number of addresses that are currently in the 
 *          AddressTable for this port. If this object has the same value as 
 *          secureNumberAddresses, then no more addresses can be authorised on this
 *          port.
 * INPUT:  unit, port,secure_number_addresses_stored.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_GetSecureNumberAddressesStored(UI32_T unit, UI32_T port, UI32_T *secure_number_addresses_stored);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetSecureMaximumAddresses
 * ---------------------------------------------------------------------
 * PURPOSE: Get the maxinum value that secureNumberAddresses 
 *          can be set to.
 * INPUT:  unit, port
 * OUTPUT: secure_maximum_addresses
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
BOOL_T NETWORKACCESS_MGR_GetSecureMaximumAddresses(UI32_T unit, UI32_T port, UI32_T *secure_maximum_addresses);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetMinimumSecureAddresses
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the minimum value that secureNumberAddresses can be set to.
 * INPUT    : port_mode
 * OUTPUT   : min_addresses
 * RETURN   : TRUE -- succeeded / FALSE -- failed.
 * NOTES    : none
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_GetMinimumSecureAddresses(UI32_T port_mode, UI32_T *min_addresses);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetUsableSecureAddresses
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the maximum value that secureNumberAddresses can be set to.
 * INPUT    : lport
 * OUTPUT   : usable_addresses
 * RETURN   : TRUE/FALSE.
 * NOTES    : 
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_GetUsableSecureAddresses(UI32_T lport, UI32_T *usable_addresses);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetUnreservedSecureAddresses
 * ---------------------------------------------------------------------
 * PURPOSE: Get the number of unreserved secure addresses
 * INPUT:   none
 * OUTPUT:  unreserved_nbr
 * RETURN:  TRUE/FALSE.
 * NOTES:   none
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_GetUnreservedSecureAddresses(UI32_T *unreserved_nbr);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetNbrOfMacWriteToAmtr
 * ---------------------------------------------------------------------
 * PURPOSE: Get the number of mac write to AMTR
 * INPUT:   none
 * OUTPUT:  mac_nbr
 * RETURN:  TRUE/FALSE.
 * NOTES:   aggregate it since SecurePortSecurityControl is enabled
 *          and reset it when it is disabled
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_GetNbrOfMacWriteToAmtr(UI32_T *mac_nbr);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_MGR_GetSecureStopStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : get secure stop status
 * INPUT    : none
 * OUTPUT   : secure_stop_status
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : Per customer's request,always return 1 for secureStop MIB node
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_MGR_GetSecureStopStatus(UI32_T *secure_stop_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_SetSecurePortSecurityControl
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
BOOL_T NETWORKACCESS_MGR_SetSecurePortSecurityControl(BOOL_T secure_port_security_control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetSecurePortSecurityControl
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
BOOL_T NETWORKACCESS_MGR_GetSecurePortSecurityControl(BOOL_T *secure_port_security_control);
 
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetRunningSecurePortSecurityControl
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default secure_port_security_control is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   None.
 * OUTPUT:  secure_port_security_control
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETWORKACCESS_MGR_GetRunningSecurePortSecurityControl(BOOL_T *secure_port_security_control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_SetSecureRadaDefaultSessionTime
 * ---------------------------------------------------------------------
 * PURPOSE: Set the default session lifetime.
 * INPUT:  secure_rada_default_session_time.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  Specifies the default session lifetime in seconds before
 *         a forwarding MAC address is re-authenticated.
 *         The default time is 1800 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_SetSecureRadaDefaultSessionTime(UI32_T secure_rada_default_session_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetSecureRadaDefaultSessionTime
 * ---------------------------------------------------------------------
 * PURPOSE: Get the default session lifetime.
 * INPUT:  None.
 * OUTPUT: secure_rada_default_session_time.
 * RETURN: TRUE/FALSE.
 * NOTES:  Specifies the default session lifetime in seconds before
 *         a forwarding MAC address is re-authenticated.
 *         The default time is 1800 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_GetSecureRadaDefaultSessionTime(UI32_T *secure_rada_default_session_time);
 
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetRunningSecureRadaDefaultSessionTime
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default secure_rada_default_session_time is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   None.
 * OUTPUT:  secure_rada_default_session_time
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETWORKACCESS_MGR_GetRunningSecureRadaDefaultSessionTime(UI32_T *secure_rada_default_session_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_SetSecureRadaHoldoffTime
 * ---------------------------------------------------------------------
 * PURPOSE: Set the holdoff time.
 * INPUT:  secure_rada_holdoff_time.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  Specifies the time in seconds before a blocked (denied) 
 *         MAC address can be re-authenticated.
 *         The default time is 60 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_SetSecureRadaHoldoffTime(UI32_T secure_rada_holdoff_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetSecureRadaHoldoffTime
 * ---------------------------------------------------------------------
 * PURPOSE: Get the holdoff time.
 * INPUT:  None.
 * OUTPUT: secure_rada_holdoff_time.
 * RETURN: TRUE/FALSE.
 * NOTES:  Specifies the time in seconds before a blocked (denied) 
 *         MAC address can be re-authenticated.
 *         The default time is 60 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_GetSecureRadaHoldoffTime(UI32_T *secure_rada_holdoff_time);
 
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetRunningSecureRadaHoldoffTime
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default secure_rada_holdoff_time is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   None.
 * OUTPUT:  secure_rada_holdoff_time
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETWORKACCESS_MGR_GetRunningSecureRadaHoldoffTime(UI32_T *secure_rada_holdoff_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_SetSecureRadaReauthenticate
 * ---------------------------------------------------------------------
 * PURPOSE: Execute RADA re-authentication.
 * INPUT:  MAC address.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  Writing a MAC address to this object causes an
 *          immediate RADA re-authentication of this address (can be on 
 *          any port). If the MAC address not currently known to RADA,
 *          it silently ignores the write.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_SetSecureRadaReauthenticate(UI8_T *mac_address);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_SetSecureRadaAuthMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set the RADA authentication mode.
 * INPUT:  secure_rada_auth_mode.
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
BOOL_T NETWORKACCESS_MGR_SetSecureRadaAuthMode(UI32_T secure_rada_auth_mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetSecureRadaAuthMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the RADA authentication mode.
 * INPUT:  None.
 * OUTPUT: secure_rada_auth_mode.
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
BOOL_T NETWORKACCESS_MGR_GetSecureRadaAuthMode(UI32_T *secure_rada_auth_mode);
 
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetRunningSecureRadaAuthMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default secure_rada_auth_mode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   None.
 * OUTPUT:  secure_rada_auth_mode
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETWORKACCESS_MGR_GetRunningSecureRadaAuthMode(UI32_T *secure_rada_auth_mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_SetSecureRadaAuthUsername
 * ---------------------------------------------------------------------
 * PURPOSE: Set the RADA authenticate username
 * INPUT:  secure_rada_auth_username.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  This is the username used for authentication requests
 *         where secureRadaAuthMode is papUsernameFixed.
 *         Default shall be 'mac'.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_SetSecureRadaAuthUsername(UI8_T *secure_rada_auth_username);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetSecureRadaAuthUsername
 * ---------------------------------------------------------------------
 * PURPOSE: Get the RADA authenticate username.
 * INPUT:  None.
 * OUTPUT: secure_rada_auth_username.
 * RETURN: TRUE/FALSE.
 * NOTES:  This is the username used for authentication requests
 *         where secureRadaAuthMode is papUsernameFixed.
 *         Default shall be 'mac'.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_GetSecureRadaAuthUsername(UI8_T *secure_rada_auth_username);
 
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetRunningSecureRadaAuthUsername
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default secure_rada_auth_username is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   None.
 * OUTPUT:  secure_rada_auth_username
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETWORKACCESS_MGR_GetRunningSecureRadaAuthUsername(UI8_T *secure_rada_auth_username);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_SetSecureRadaAuthPassword
 * ---------------------------------------------------------------------
 * PURPOSE: Set the RADA authenticate password.
 * INPUT:  secure_rada_auth_password.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  This is the password used for authentication requests
 *         where secureRadaAuthMode is papUsernameFixed.
 *         Default shall be a null string.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_SetSecureRadaAuthPassword(UI8_T *secure_rada_auth_password);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetSecureRadaAuthPassword
 * ---------------------------------------------------------------------
 * PURPOSE: Get the RADA authenticate password.
 * INPUT:  None.
 * OUTPUT: secure_rada_auth_password.
 * RETURN: TRUE/FALSE.
 * NOTES:  This is the password used for authentication requests
 *         where secureRadaAuthMode is papUsernameFixed.
 *         Default shall be a null string.
 * ---------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_GetSecureRadaAuthPassword(UI8_T *secure_rada_auth_password);
 
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetRunningSecureRadaAuthPassword
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default secure_rada_auth_password is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   None.
 * OUTPUT:  secure_rada_auth_password
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETWORKACCESS_MGR_GetRunningSecureRadaAuthPassword(UI8_T *secure_rada_auth_password);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_SetPaePortAssignEnable
 * ---------------------------------------------------------------------
 * PURPOSE: Set 3Com PAE port assignment configuration control 
 * INPUT:  lport,
 *         pae_port_assign_enable = 
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
BOOL_T NETWORKACCESS_MGR_SetPaePortAssignEnable(UI32_T lport,UI32_T pae_port_assign_enable);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetPaePortAssignEnable
 * ---------------------------------------------------------------------
 * PURPOSE: Get 3Com PAE port assignment configuration control 
 * INPUT:  lport.
 * OUTPUT: pae_port_assign_enable = 
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
BOOL_T NETWORKACCESS_MGR_GetPaePortAssignEnable(UI32_T lport,UI32_T *pae_port_assign_enable);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_MGR_GetRunningPaePortAssignEnable
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default a3Com_pae_port_assign_enable is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   lport.
 * OUTPUT: pae_port_assign_enable = 
 *          VAL_a3ComPaePortAssignEnable_true	  1L
 *          VAL_a3ComPaePortAssignEnable_false	2L 
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETWORKACCESS_MGR_GetRunningPaePortAssignEnable(UI32_T lport,UI32_T *pae_port_assign_enable);

/******************************************
 * For 3COM 3FC-458R.mib and 3FC-407R.mib * 
 ******************************************/ 
/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_GetSecurePortEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get secure port entry by the unit and the port.
 * INPUT    : unit : secureSlotIndex.
 *            port : securePortIndex.
 * OUTPUT   : entry : The secure port entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : The unit and port base on 1.
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_GetSecurePortEntry(UI32_T unit,UI32_T port,NETWORKACCESS_SecurePortEntry_T *entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_GetNextSecurePortEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get secure port entry by the unit and the port.
 * INPUT    : unit : secureSlotIndex.
 *            port : securePortIndex.
 * OUTPUT   : entry : The secure port entry.
 * RETURN   : TRUE/FALSE
 * NOTES    :  The unit and port base on 1.
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_GetNextSecurePortEntry(UI32_T *unit,UI32_T *port,NETWORKACCESS_SecurePortEntry_T *entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_GetSecurePortInfo
 *-----------------------------------------------------------------------------------
 * PURPOSE  : get specific port's detail information
 * INPUT    : unit, port
 * OUTPUT   : detail_info
 * RETURN   : TRUE -- succeeded / FALSE -- failed
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_GetSecurePortInfo(UI32_T unit, UI32_T port, NETWORKACCESS_PortDetailData_T *detail_info);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_GetNextSecurePortInfo
 *-----------------------------------------------------------------------------------
 * PURPOSE  : get specific next port's detail information
 * INPUT    : unit, port
 * OUTPUT   : unit, port, detail_info
 * RETURN   : TRUE -- succeeded / FALSE -- failed
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_GetNextSecurePortInfo(UI32_T *unit, UI32_T *port, NETWORKACCESS_PortDetailData_T *detail_info);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_SetSecurePortMode
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will set securePortMode by the unit and the port.
 * INPUT    : unit : secureSlotIndex.
 *            port : securePortIndex.
 *            secure_port_mode:securePortMode.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    :  The unit and port base on 1.
 *-----------------------------------------------------------------------------------
 */
/*BOOL_T NETWORKACCESS_MGR_SetSecurePortMode(UI32_T unit,UI32_T port,UI32_T secure_port_mode);*/

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_SetSecureNeedToKnowMode
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will set secureNeedToKnowMode by the unit and the port.
 * INPUT    : unit : secureSlotIndex.
 *            port : securePortIndex.
 *            need_to_know_mode :secureNeedToKnowMode
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    :  The unit and port base on 1.
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_SetSecureNeedToKnowMode(UI32_T unit,UI32_T port,UI32_T need_to_know_mode);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_SetSecureIntrusionAction
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will set secureIntrusionAction by the unit and the port.
 * INPUT    : unit : secureSlotIndex.
 *            port : securePortIndex.
 *            intrusion_action:secureIntrusionAction
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    :  The unit and port base on 1.
 *-----------------------------------------------------------------------------------
 */
/*BOOL_T NETWORKACCESS_MGR_SetSecureIntrusionAction(UI32_T unit,UI32_T port,UI32_T intrusion_action);*/

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_SetSecureNumberAddresses
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will set secureNumberAddresses by the unit and the port.
 * INPUT    : unit : secureSlotIndex.
 *            port : securePortIndex.
 *            number:secureNumberAddresses
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    :  The unit and port base on 1.
 *-----------------------------------------------------------------------------------
 */
/*BOOL_T NETWORKACCESS_MGR_SetSecureNumberAddresses(UI32_T unit,UI32_T port,UI32_T number);*/

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_CreateSecureAddrEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : create a mac entry
 * INPUT    : unit, port, mac, is_active
 * OUTPUT   : reason
 * RETURN   : TRUE -- succeeded / FALSE -- failed
 * NOTES    : reason is meaningful only when return FALSE
 *            allow to pass NULL pointer if don't care reason
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_CreateSecureAddrEntry(UI32_T unit, UI32_T port, UI8_T *mac, BOOL_T is_active, NETWORKACCESS_FailureReason_T *reason);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_DestroySecureAddrEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : destroy a mac entry
 * INPUT    : unit, port, mac
 * OUTPUT   : none
 * RETURN   : TRUE -- succeeded / FALSE -- failed
 * NOTES    : if mac is not existed, return FALSE
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_DestroySecureAddrEntry(UI32_T unit, UI32_T port, UI8_T *mac);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_DoesPortAllowCreateSecureAddrEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : whether the specified port allow to create mac or not
 * INPUT    : unit, port
 * OUTPUT   : none
 * RETURN   : TRUE -- yes / FALSE -- no
 * NOTES    : none
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_DoesPortAllowCreateSecureAddrEntry(UI32_T unit, UI32_T port);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_GetSecureAddressEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get secure address entry entry by the unit,the port and the mac_address.
 * INPUT    : unit : secureSlotIndex.
 *            port : securePortIndex.
 *            mac_address  : secureAddrMAC.
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : The unit and port base on 1.
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_GetSecureAddressEntry(UI32_T unit,UI32_T port,UI8_T *mac_address,NETWORKACCESS_SecureAddressEntry_T *entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_GetNextSecureAddressEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get next secure address entry by the unit and the port and the mac_address.
 * INPUT    : unit : secureSlotIndex.
 *            port : securePortIndex.
 *            mac_address  : secureAddrMAC.
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : The unit and the port base on 1.
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_GetNextSecureAddressEntry(UI32_T *unit, UI32_T *port, UI8_T *mac_address, NETWORKACCESS_SecureAddressEntry_T *entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_GetNextRunningSecureAddressEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : get next running secure address entry by the unit and the port and the mac_address.
 * INPUT    : unit, port, mac_address
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : The unit and the port base on 1.
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_GetNextRunningSecureAddressEntry(UI32_T *unit, UI32_T *port, UI8_T *mac_address, NETWORKACCESS_SecureAddressEntry_T *entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_SetSecureAddrRowStatus
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will set secure port entry  by the unit and the port and the mac_address.
 * INPUT    : unit : secureSlotIndex.
 *            port : securePortIndex.
 *            mac_address  : secureAddrMAC.
 *            status : secureAddrRowStatus
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : The unit and the port base on 1.
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_SetSecureAddrRowStatus(UI32_T unit, UI32_T port, UI8_T *mac_address,UI32_T status);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_GetSecurePortSecurityControl
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get securePortSecurityControl.
 * INPUT    : None.
 * OUTPUT   : control : securePortSecurityControl.
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 *-----------------------------------------------------------------------------------
 */
/*BOOL_T NETWORKACCESS_MGR_GetSecurePortSecurityControl(UI32_T *control);*/

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_SetSecurePortSecurityControl
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will set securePortSecurityControl.
 * INPUT    : control : securePortSecurityControl.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 *-----------------------------------------------------------------------------------
 */
/*BOOL_T NETWORKACCESS_MGR_SetSecurePortSecurityControl(UI32_T control);*/

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_GetSecureOUIEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get secure OUI entry by the unit and the OUI.
 * INPUT    : unit : secureSlotIndex.
 *            OUI  : secureOUI.
 * OUTPUT   : entry : secure OUI entry.
 * RETURN   : TRUE/FALSE
 * NOTES    : The unit bases on 1.
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_GetSecureOUIEntry(UI32_T unit, UI8_T *OUI ,NETWORKACCESS_SecureOUIEntry_T *entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_GetNextSecureOUIEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get next secure OUI entry by the unit and the OUI.
 * INPUT    : unit : secureSlotIndex.
 *            OUI  : secureOUI.
 * OUTPUT   : entry : The secure address entry.
 * RETURN   : TRUE/FALSE
 * NOTES    :  The unit base on 1.
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_GetNextSecureOUIEntry(UI32_T unit, UI8_T *OUI ,NETWORKACCESS_SecureOUIEntry_T *entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_SetSecureOUIRowStatus
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will set SecureOUIRowStatus by the unit and the OUI.
 * INPUT    : unit : secureSlotIndex.
 *            OUI  : secureOUI.
 *            status:SecureOUIRowStatus.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    :  The unit base on 1.
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_SetSecureOUIRowStatus(UI32_T unit, UI8_T *OUI ,UI32_T status);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_GetSecurePortVlanMembershipList
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get securePortVlanMembershipList.
 * INPUT    : None.
 * OUTPUT   : number_ship_list : securePortVlanMembershipList.
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_GetSecurePortVlanMembershipList(UI8_T *number_ship_list);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_GetSecureRadaDefaultSessionTime
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get secureRadaDefaultSessionTime.
 * INPUT    : None.
 * OUTPUT   : time : secureRadaDefaultSessionTime.
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 *-----------------------------------------------------------------------------------
 */
/*BOOL_T NETWORKACCESS_MGR_GetSecureRadaDefaultSessionTime(UI32_T *time);*/

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_SetSecureRadaDefaultSessionTime
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will set secureRadaDefaultSessionTime.
 * INPUT    : time : secureRadaDefaultSessionTime.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 *-----------------------------------------------------------------------------------
 */
/*BOOL_T NETWORKACCESS_MGR_SetSecureRadaDefaultSessionTime(UI32_T time);*/

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_GetSecureRadaHoldoffTime
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get secureRadaHoldoffTime.
 * INPUT    : None.
 * OUTPUT   : time : secureRadaHoldoffTime.
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 *-----------------------------------------------------------------------------------
 */
/*BOOL_T NETWORKACCESS_MGR_GetSecureRadaHoldoffTime(UI32_T *time);*/

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_SetSecureRadaHoldoffTime
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will set secureRadaHoldoffTime.
 * INPUT    : time : secureRadaHoldoffTime.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 *-----------------------------------------------------------------------------------
 */
/*BOOL_T NETWORKACCESS_MGR_SetSecureRadaHoldoffTime(UI32_T time);*/

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_SetSecureRadaReauthenticate
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will set secureRadaReauthenticate.
 *            Writing a MAC address to this object causes an
 *            immediate RADA re-authentication of this address (can be on 
 *            any port). If the MAC address not currently known to RADA,
 *            it silently ignores the write.
 * INPUT    : mac_address : secureRadaReauthenticate.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 *-----------------------------------------------------------------------------------
 */
/*BOOL_T NETWORKACCESS_MGR_SetSecureRadaReauthenticate(UI8_T *mac_address);*/

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_GetSecureRadaAuthMode
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get secureRadaAuthMode.
 * INPUT    : None.
 * OUTPUT   : auth_mode : secureRadaAuthMode.
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 *-----------------------------------------------------------------------------------
 */
/*BOOL_T NETWORKACCESS_MGR_GetSecureRadaAuthMode(UI32_T *auth_mode);*/

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_SetSecureRadaAuthMode
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will set secureRadaAuthMode.
 * INPUT    : auth_mode : secureRadaAuthMode.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 *-----------------------------------------------------------------------------------
 */
/*BOOL_T NETWORKACCESS_MGR_SetSecureRadaAuthMode(UI32_T auth_mode);*/

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_GetSecureRadaAuthUsername
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get secureRadaAuthUsername.
 * INPUT    : None.
 * OUTPUT   : user_name : secureRadaAuthUsername.
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 *-----------------------------------------------------------------------------------
 */
/*BOOL_T NETWORKACCESS_MGR_GetSecureRadaAuthUsername(UI8_T *user_name);*/

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_SetSecureRadaAuthUsername
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will set secureRadaAuthUsername.
 * INPUT    : user_name : secureRadaAuthUsername.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 *-----------------------------------------------------------------------------------
 */
/*BOOL_T NETWORKACCESS_MGR_SetSecureRadaAuthUsername(UI8_T *user_name);*/

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_SetSecureRadaAuthPassword
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will set secureRadaAuthPassword.
 *            This is the password used for authentication requests
 *            where secureRadaAuthMode is papUsernameFixed.
 *            Default shall be a null string.
 * INPUT    : password : secureRadaAuthPassword.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 *-----------------------------------------------------------------------------------
 */
/*BOOL_T NETWORKACCESS_MGR_SetSecureRadaAuthPassword(UI8_T *password);*/

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_GetPaePortEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get PAE port entry by the port.
 * INPUT    : lport : dot1xPaePortNumber.
 * OUTPUT   : entry: PaePortEntry.
 * RETURN   : TRUE/FALSE
 * NOTES    : The port bases on 1.
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_GetPaePortEntry(UI32_T lport,NETWORKACCESS_PaePortEntry_T *entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_GetNextPaePortEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will get next PAE port entry by the port.
 * INPUT    : lport : dot1xPaePortNumber.
 * OUTPUT   : entry: PaePortEntry.
 * RETURN   : TRUE/FALSE
 * NOTES    :  The port base on 1.
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETWORKACCESS_MGR_GetNextPaePortEntry(UI32_T *lport,NETWORKACCESS_PaePortEntry_T *entry);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_SetPaePortAssignEnable
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will set PaePortAssignEnable by the port.
 * INPUT    : port : dot1xPaePortNumber. 
 *            attribute : PaePortAssignEnable.
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTES    : The port base on 1.
 *-----------------------------------------------------------------------------------
 */
/*BOOL_T NETWORKACCESS_MGR_SetPaePortAssignEnable(UI32_T port,UI32_T attribute);*/

#if (NETWORKACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_MGR_SetDebugFlag
 *-------------------------------------------------------------------------
 * PURPOSE  : set backdoor debug flag
 * INPUT    : debug_flag
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_MGR_SetDebugFlag(UI32_T debug_flag);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_MGR_GetSecurePortIsBlocked
 *-------------------------------------------------------------------------
 * PURPOSE  : get is_blocked flag
 * INPUT    : lport
 * OUTPUT   : is_blocked
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_MGR_GetSecurePortIsBlocked(UI32_T lport, BOOL_T *is_blocked);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_MGR_ShowStateMachineStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : show state machine's status
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_MGR_ShowStateMachineStatus(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_MGR_ShowSecureAddressByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : show secure address by mac_index
 * INPUT    : mac_index
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_MGR_ShowSecureAddressByIndex(UI32_T mac_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_MGR_ShowOtherDebugData
 *-------------------------------------------------------------------------
 * PURPOSE  : show other debug data
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_MGR_ShowOtherDebugData();

#endif /* NETWORKACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE */

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_HandleHotInsertion
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is inserted at a time.
 *-----------------------------------------------------------------------------------
 */
void NETWORKACCESS_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_MGR_HandleHotRemoval
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is removed at a time.
 *-----------------------------------------------------------------------------------
 */ 
void NETWORKACCESS_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_MGR_Set_Dot1xPortControlMode
 *-------------------------------------------------------------------------
 * PURPOSE  : set port control mode of 1X configuration
 * INPUT    : lport, control_mode
 * OUTPUT   : none
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : control_mode (define in leaf_Ieee8021x.h):
 *                  VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized
 *                  VAL_dot1xAuthAuthControlledPortControl_forceAuthorized
 *                  VAL_dot1xAuthAuthControlledPortControl_auto
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_MGR_Set_Dot1xPortControlMode(UI32_T lport,UI32_T control_mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETWORKACCESS_MGR_IsDot1xUserLogon
 *-------------------------------------------------------------------------
 * PURPOSE  : whether dot1x user logon this port
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE -- yes, FALSE -- no
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T NETWORKACCESS_MGR_IsDot1xUserLogon(UI32_T lport);

/* FUNCTION NAME - NETWORKACCESS_MGR_ProvisionComplete
 * PURPOSE  : This function sets the component to temporary transition mode           
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void  NETWORKACCESS_MGR_ProvisionComplete(void);

/* FUNCTION NAME - NETWORKACCESS_MGR_ProcessProvisionComplete
 * PURPOSE  : This function do thing if need when provision complete(ex.delete permanent MAC for security port)           
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void  NETWORKACCESS_MGR_ProcessProvisionComplete(void);

#endif /*NETWORKACCESS_MGR_H*/