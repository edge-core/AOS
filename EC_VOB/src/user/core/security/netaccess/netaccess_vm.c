/* Project NAME:  NETACCESS_VM.C
* PURPOSE:
* Net Access state machine
*
*REASON:
*      Description:
*      CREATOR:      Ricky Lin
*      Date         2006/01/27
*
* Copyright(C)      Accton Corporation, 2004
*/
/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_NETACCESS == TRUE)
#include <stdio.h>
#include <string.h>
#include "netaccess_vm.h"
#include "netaccess_om.h"
#include "l_mm.h"
#include "sys_time.h"
#include "swctrl.h"
#include "swctrl_pmgr.h"
#include "amtr_mgr.h"
#include "amtr_om.h"
#include "l2mux_mgr.h"
#include "vlan_pmgr.h"
#include "vlan_pom.h"
#include "vlan_om.h"
#include "vlan_lib.h"
#include "1x_om.h"
#include "1x_mgr.h"
#include "1x_types.h"
#include "dot1x_sm_auth.h"
#include "radius_pmgr.h"
#include "portauthsrvc_mgr.h"
#include "trap_event.h"
#include "sys_module.h"
#include "snmp_pmgr.h"
#include "dot1x_vm.h"

#if (SYS_CPNT_ADD == TRUE)
#include "amtr_type.h"
#include "add_om.h"
#include "add_mgr.h"
#endif

#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
#include "l4_pmgr.h"
#endif

#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)
#include "psec_om.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define NETACCESS_VM_EAPOL_START_CODE               1
#define NETACCESS_VM_EAP_FRAME_TYPE                 0x888e
#define NETACCESS_VM_DISABLE_PORT_SECONDS           20  /* 3Com-458, page 8, 2.4 - 4. Disable Port Temporarily */
#define NETACCESS_VM_VIOLATION_TRAP_HOLDOFF_SECONDS 5   /* 3Com-458, page 17, Only one violation trap SHALL be sent per port in any five-second period */
#define NETACCESS_VM_MAX_DIGIT_NBR_OF_VID           4   /* max VLAN ID is 4096 */
#define NETACCESS_VM_MAX_LEN_OF_AUTH_USERNAME       10L
#define NETACCESS_VM_MAX_LEN_OF_AUTH_PASSWORD       10L

/* For error message
 */
#define NETACCESS_VM_DERR                           (NETACCESS_OM_DEBUG_VM_ERR  \
                                                    | NETACCESS_OM_DEBUG_VM_IFO \
                                                    | NETACCESS_OM_DEBUG_VM_RST \
                                                    | NETACCESS_OM_DEBUG_VM_TMR \
                                                    | NETACCESS_OM_DEBUG_VM_TRC)

/* For important message
 */
#define NETACCESS_VM_DIFO                           (NETACCESS_OM_DEBUG_VM_IFO  \
                                                    | NETACCESS_OM_DEBUG_VM_RST \
                                                    | NETACCESS_OM_DEBUG_VM_TRC)

/* For trace message
 */
#define NETACCESS_VM_DTRC                           (NETACCESS_OM_DEBUG_VM_TRC  \
                                                    | NETACCESS_OM_DEBUG_VM_RST)

/* For timer message
 */
#define NETACCESS_VM_DTMR                           (NETACCESS_OM_DEBUG_VM_TMR)

/* MACRO FUNCTION DECLARATIONS
 */
#define NETACCESS_VM_RUN_PORT_MODE_CHANGE_SM(lport, sm) \
    if ((FALSE == NETACCESS_VM_TranPortModeChangeSM(lport, sm)) || \
        (FALSE == NETACCESS_VM_ExecPortModeChangeSM(lport, sm))) { \
        return FALSE; \
    }

#define NETACCESS_VM_RUN_NO_RESTRICTION_SM(lport, sm) \
    if ((FALSE == NETACCESS_VM_TranNoRestrictionSM(lport, sm)) || \
        (FALSE == NETACCESS_VM_ExecNoRestrictionSM(lport, sm))) { \
        return FALSE; \
    }

#define NETACCESS_VM_RUN_MAC_AUTHENTICATION_SM(lport, sm) \
    if ((FALSE == NETACCESS_VM_TranMacAuthenticationSM(lport, sm)) || \
        (FALSE == NETACCESS_VM_ExecMacAuthenticationSM(lport, sm))) { \
        return FALSE; \
    }

#define NETACCESS_VM_RUN_PORT_SECURITY_SM(lport, sm) \
    if ((FALSE == NETACCESS_VM_TranPortSecuritySM(lport, sm)) || \
        (FALSE == NETACCESS_VM_ExecPortSecuritySM(lport, sm))) { \
        return FALSE; \
    }

#define NETACCESS_VM_RUN_DOT1X_SM(lport, sm) \
    if ((FALSE == NETACCESS_VM_TranDot1xSM(lport, sm)) || \
        (FALSE == NETACCESS_VM_ExecDot1xSM(lport, sm))) { \
        return FALSE; \
    }

#define NETACCESS_VM_RUN_SECURE_SM(lport, sm) \
    if ((FALSE == NETACCESS_VM_TranSecureSM(lport, sm)) || \
        (FALSE == NETACCESS_VM_ExecSecureSM(lport, sm))) { \
        return FALSE; \
    }

#define NETACCESS_VM_RUN_MAC_ADDRESS_OR_USER_LOGIN_SECURE_SM(lport, sm) \
    if ((FALSE == NETACCESS_VM_TranMacAddressOrUserLoginSecureSM(lport, sm)) || \
        (FALSE == NETACCESS_VM_ExecMacAddressOrUserLoginSecureSM(lport, sm))) { \
        return FALSE; \
    }

#define NETACCESS_VM_DEBUG_PRINT_FUNCTION_START()                           \
    NETACCESS_DBG(NETACCESS_VM_DTRC, "Start ...")

#define NETACCESS_VM_DEBUG_PRINT_FUNCTION_END()                             \
    NETACCESS_DBG(NETACCESS_VM_DTRC, "End")

#define NETACCESS_VM_DEBUG_PRINT_TRACE(fmt, args...)                        \
    NETACCESS_DBG(NETACCESS_VM_DTRC, fmt, ##args)

#define NETACCESS_VM_DEBUG_PRINT_IMPORTANT(fmt, args...)                    \
    NETACCESS_DBG(NETACCESS_VM_DIFO, fmt, ##args)

#define NETACCESS_VM_DEBUG_PRINT_ERROR(fmt, args...)                        \
    NETACCESS_DBG(NETACCESS_VM_DERR, fmt, ##args)

#define NETACCESS_VM_DEBUG_PRINT_TIMER(fmt, args...)                        \
    NETACCESS_DBG(NETACCESS_VM_DTMR, fmt, ##args)

#define NETACCESS_VM_RETURNED_VLAN(sm)                                      \
    (                                                                       \
    (sm->port_security_sm.dot1x_msg) ?                                      \
    sm->port_security_sm.dot1x_msg->m_dot1x_data->authorized_vlan_list :    \
    (sm->port_security_sm.radius_msg) ?                                     \
    sm->port_security_sm.radius_msg->m_radius_data->authorized_vlan_list :  \
    NULL                                                                    \
    )

#define NETACCESS_VM_RETURNED_QOS(sm)                                       \
    (                                                                       \
    (sm->port_security_sm.dot1x_msg) ?                                      \
    sm->port_security_sm.dot1x_msg->m_dot1x_data->authorized_qos_list :     \
    (sm->port_security_sm.radius_msg) ?                                     \
    sm->port_security_sm.radius_msg->m_radius_data->authorized_qos_list :   \
    NULL                                                                    \
    )

/* DATA TYPE DECLARATIONS
 */
enum
{
    NETACCESS_VM_DOT1X = 0,         /* the number is sequence of execution  */
    NETACCESS_VM_RUNNING_PORT_MODE,
    NETACCESS_VM_LINK_DETECTION,
    NETACCESS_VM_NUMBER_OF_LINK_UP_HANDLER,
    NETACCESS_VM_NUMBER_OF_LINK_DOWN_HANDLER = NETACCESS_VM_NUMBER_OF_LINK_UP_HANDLER
};

typedef BOOL_T (*NETACCESS_VM_AllowVlanChangeFuncPtr_T)(UI32_T lport);
typedef BOOL_T (*NETACCESS_VM_AllowReauthFuncPtr_T)(NETACCESS_OM_StateMachine_T *state_machine, UI32_T mac_index);
typedef BOOL_T (*NETACCESS_VM_EnterPortModeFuncPtr_T)(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);

typedef void (*NETACCESS_VM_PortLinkUpFun_T)(UI32_T);
typedef void (*NETACCESS_VM_PortLinkDownFun_T)(UI32_T);

typedef struct NETACCESS_EAPOL_START_PACKET_S
{
    UI8_T   version;
    UI8_T   type;
    UI16_T  length;
} NETACCESS_EAPOL_START_PACKET_T;

typedef struct NETACCESS_VM_UpdateFlag_S
{
    UI8_T   vlan            :1;
    UI8_T   qos             :1;
    UI8_T   session_time    :1;
    UI8_T   reserved_bits   :5;
} NETACCESS_VM_UpdateFlag_T;

typedef struct NETACCESS_VM_PortModeFuncPtr_S
{
    NETACCESS_VM_AllowVlanChangeFuncPtr_T   allow_vlan_change;
    NETACCESS_VM_AllowReauthFuncPtr_T       allow_reauth;
    NETACCESS_VM_EnterPortModeFuncPtr_T     enter_port_mode;
} NETACCESS_VM_PortModeFuncPtr_T;

typedef struct NETACCESS_VM_PortMove_S
{
    BOOL_T   static2staic;    /* from static to static */
    BOOL_T   static2dynamic;  /* from static to dynamic */
    BOOL_T   dynamic2staic;   /* from dynamic to static */
    BOOL_T   dynamic2dynamic; /* from dynamic to dynamic */
} NETACCESS_VM_PortMove_T;

typedef struct NETACCESS_VM_PortMode_S
{
    UI32_T   is_allow_add_secure_address:1; /* is allow secure address */
    UI32_T   is_allow_exec_intrusion_action:1; /* is allow execute intrusion action */
    UI32_T   is_allow_update_mac_entry:1; /* is allow update mac entry */
    UI32_T   is_allow_kickout_dot1x_address:1; /* is allow kick out dot1x mac address */
    UI32_T   reserved:28; /* reserved */
} NETACCESS_VM_PortMode_T;

typedef struct
{
    UI32_T vid;
    SYS_CALLBACK_MGR_AuthenticatePacket_Result_T result;
}NETACCESS_VM_AuthResult_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T NETACCESS_VM_LocalInitializeVM();

/* functions for port mode change state machine */
static BOOL_T NETACCESS_VM_EnterSecurePortMode(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);
static BOOL_T NETACCESS_VM_ExitSecurePortMode(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);

/* state machine functions */
static BOOL_T NETACCESS_VM_RunSecurePortModeSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);

static BOOL_T NETACCESS_VM_TranPortModeChangeSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);
static BOOL_T NETACCESS_VM_ExecPortModeChangeSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);

static BOOL_T NETACCESS_VM_TranNoRestrictionSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);
static BOOL_T NETACCESS_VM_ExecNoRestrictionSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);

static BOOL_T NETACCESS_VM_TranSecureSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);
static BOOL_T NETACCESS_VM_ExecSecureSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);

static BOOL_T NETACCESS_VM_TranMacAddressOrUserLoginSecureSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);
static BOOL_T NETACCESS_VM_ExecMacAddressOrUserLoginSecureSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);

static BOOL_T NETACCESS_VM_TranMacAuthenticationSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);
static BOOL_T NETACCESS_VM_ExecMacAuthenticationSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);
static BOOL_T NETACCESS_VM_TranPortSecuritySM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);
static BOOL_T NETACCESS_VM_ExecPortSecuritySM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);
static BOOL_T NETACCESS_VM_TranDot1xSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);
static BOOL_T NETACCESS_VM_ExecDot1xSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);

/* common functions */
static BOOL_T NETACCESS_VM_LocalEnableAutoLearning(UI32_T lport);
static BOOL_T NETACCESS_VM_LocalDisableAutoLearningAndTrap2CPU(
    UI32_T lport,
    UI32_T runn_port_mode
    );
static BOOL_T NETACCESS_VM_LocalCheckAutoPortMoveIssue(UI32_T original_port_mode, UI32_T new_port_mode, BOOL_T is_from_staic, BOOL_T is_to_static);
static BOOL_T NETACCESS_VM_LocalIsPortOperUp(UI32_T lport);
static BOOL_T NETACCESS_VM_LocalPortDown(UI32_T lport);

static BOOL_T NETACCESS_VM_LocalGetNextMacFromAmtr(const UI8_T *mac, UI32_T *lport, UI32_T *vid);

static BOOL_T NETACCESS_VM_LocalTriggerDot1xSendEapRequest(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);
static BOOL_T NETACCESS_VM_LocalForwardEapPacketToDot1x(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);
#if 0
static BOOL_T NETACCESS_VM_LocalNotifyDot1xSendEapResult(UI32_T lport, UI8_T *mac, BOOL_T is_succeeded);
#endif

static BOOL_T NETACCESS_VM_LocalTriggerRadaDoAuthentication(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);
static BOOL_T NETACCESS_VM_LocalLearnNewMac(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);
static BOOL_T NETACCESS_VM_LocalIsSecureAddressTableFull(UI32_T lport, NETACCESS_OM_SecureMacEntry_T* mac_entry_p);
static BOOL_T NETACCESS_VM_LocalIsMacAuthMaxMacCountReached(UI32_T lport);
#if (SYS_CPNT_DOT1X == TRUE)
static BOOL_T NETACCESS_VM_LocalIsDot1xMultiHostMaxMacCountReached(UI32_T lport);
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */
static BOOL_T NETACCESS_VM_LocalIsDot1xMaxMacCountReached(UI32_T lport);
static BOOL_T NETACCESS_VM_LocalLearnPreauthenticatedMac(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);
static BOOL_T NETACCESS_VM_LocalCreateNewPreauthMac(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);
static BOOL_T NETACCESS_VM_LocalAddAuthenticatingMacToAmtr(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);
static BOOL_T NETACCESS_VM_LocalDeleteAuthenticatingMac(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);
static BOOL_T NETACCESS_VM_LocalExecIntrusionAction(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);
static BOOL_T NETACCESS_VM_LocalExecMacAuthenticationIntrusionAction(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);

static BOOL_T NETACCESS_VM_LocalAuthorizeExistedMac(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);

static BOOL_T NETACCESS_VM_LocalEnterAuthenticating(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);

static void NETACCESS_VM_LocalEnterIdelState(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);

static BOOL_T NETACCESS_VM_LocalCollectMacEntry(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine, NETACCESS_OM_SecureMacEntry_T *mac_entry);

static const char* NETACCESS_VM_LocalStringState(NETACCESS_OM_StateMachineStatus_T state);
static void NETACCESS_VM_LocalPrintState(NETACCESS_OM_StateMachineStatus_T state);
static void NETACCESS_VM_LocalPrintMacAddr(const UI8_T *addr);

static BOOL_T NETACCESS_VM_LocalSendViolationTrap(UI32_T lport,UI8_T *mac);

static BOOL_T NETACCESS_VM_LocalIsValidVlanList(UI32_T lport, const char *str, NETACCESS_OM_StateMachine_T *sm_p);
static void NETACCESS_VM_LocalInitializeFunctionPointer(void);
static BOOL_T NETACCESS_VM_LocalIsPortModeAllowVlanChange(UI32_T lport);
static BOOL_T NETACCESS_VM_LocalIsPortAllowVlanChange(UI32_T lport);
static BOOL_T NETACCESS_VM_LocalIsSmAllowAddressReauth(NETACCESS_OM_StateMachine_T *state_machine, UI32_T mac_index);
static BOOL_T NETACCESS_VM_LocalEnterPortModeEnableAutoLearn(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine);

static BOOL_T NETACCESS_VM_LocalEnterMacAuthenticationMode(
    UI32_T lport,
    NETACCESS_OM_StateMachine_T *state_machine
);

static BOOL_T NETACCESS_VM_LocalEnterPortSecurityMode(
    UI32_T lport,
    NETACCESS_OM_StateMachine_T *state_machine
);

static BOOL_T NETACCESS_VM_LocalEnterSecureMode(
    UI32_T lport,
    NETACCESS_OM_StateMachine_T *state_machine
);

static BOOL_T NETACCESS_VM_LocalEnterMacAddressOrUserSecureMode(
    UI32_T lport,
    NETACCESS_OM_StateMachine_T *state_machine
);

static BOOL_T NETACCESS_VM_LocalEnterDot1xMode(
    UI32_T lport,
    NETACCESS_OM_StateMachine_T *state_machine
);

static BOOL_T NETACCESS_VM_LocalExitDot1xMode(
    UI32_T lport,
    NETACCESS_OM_StateMachine_T *state_machine
);


static BOOL_T NETACCESS_VM_UserLogin(UI32_T lport, NETACCESS_OM_StateMachine_T *sm_p);

static BOOL_T NETACCESS_VM_LocalIsNeedToJoinGuestVlan(UI32_T lport);
#if (SYS_CPNT_DOT1X == TRUE)
static BOOL_T NETACCESS_VM_LocalIsNeedToJoinAuthFailVlan(UI32_T lport);
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */
static BOOL_T NETACCESS_VM_LocalHaveDot1xMac(UI32_T lport);
static BOOL_T NETACCESS_VM_LocalSetToAutoVlan(UI32_T lport, const char *str);
static BOOL_T NETACCESS_VM_LeaveGuestVlanDisableAutoLearningAndTrap2Cpu(UI32_T lport, NETACCESS_OM_StateMachine_T *sm_p);
static BOOL_T NETACCESS_VM_LocalEnterDot1xAuthFailedState(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine_p);

static BOOL_T NETACCESS_VM_LocalIsValidQosProfiles(
    UI32_T lport,
    const char *str,
    NETACCESS_OM_StateMachine_T *sm_p
    );
static BOOL_T NETACCESS_VM_LocalApplyQosProfiles(
    UI32_T lport,
    const char *str,
    NETACCESS_OM_StateMachine_T *sm_p
    );
static BOOL_T NETACCESS_VM_LocalResetQos2Default(UI32_T lport);

static BOOL_T NETACCESS_VM_LocalLeaveAuthorizedVlan(UI32_T lport);

static BOOL_T NETACCESS_VM_LocalIsVoiceVlanEnabled(UI32_T lport);
static BOOL_T NETACCESS_VM_LocalIsVoiceVlanId(UI32_T vid);
static BOOL_T NETACCESS_VM_LocalByPassToVoiceVlan(UI32_T vid, UI8_T *mac, UI32_T lport);
static BOOL_T NETACCESS_VM_LocalDeleteSecureMacAddr(NETACCESS_OM_SecureMacEntry_T *mac_entry_p);

static UI32_T NETACCESS_VM_SecureAddrSourceType();
static BOOL_T NETACCESS_VM_LocalAgingModeOfSecureAddress();
static AMTR_TYPE_AddressLifeTime_T NETACCESS_VM_SecureAddrLifeTime(BOOL_T sec_aging);

static BOOL_T NETACCESS_VM_NotifyDot1xManagerSetupResult(UI32_T lport, NETACCESS_OM_StateMachine_T *sm_p, BOOL_T success);

/* link status change handlers
 */
#if (SYS_CPNT_DOT1X == TRUE)
static void NETACCESS_VM_Dot1x_ProcessLinkUpEvent(UI32_T lport);
static void NETACCESS_VM_Dot1x_ProcessLinkDownEvent(UI32_T lport);
#endif

#if (SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
static void NETACCESS_VM_LinkDetection_ProcessLinkUpEvent(UI32_T lport);
static void NETACCESS_VM_LinkDetection_ProcessLinkDownEvent(UI32_T lport);
static void NETACCESS_VM_LinkDetection_DoAction(UI32_T lport, UI32_T oper_status, UI32_T link_detection_mode, UI32_T link_detection_action);
static void NETACCESS_VM_LinkDetection_SendTrap(UI32_T lport, UI32_T oper_status, UI32_T link_detection_mode, UI32_T link_detection_action);
static void NETACCESS_VM_LinkDetection_Shutdown(UI32_T lport);
#endif

static void NETACCESS_VM_ProcessLinkDown(UI32_T lport);

static void NETACCESS_VM_LocalUpdateAuthResultToCookie(
    NETACCESS_VM_AuthResult_T *auth_result_p,
    NETACCESS_OM_StateMachine_T *updated_sm_p);

static void NETACCESS_VM_LocalPassCookieToNextCSC(
    UI32_T lport,
    NETACCESS_OM_StateMachine_T *state_machine_p,
    NETACCESS_VM_AuthResult_T *auth_result_p);

/* STATIC VARIABLE DECLARATIONS
 */
static NETACCESS_VM_PortModeFuncPtr_T port_mode_fun_p[NETACCESS_PORTMODE_MAX-1];
static NETACCESS_VM_PortLinkUpFun_T netaccess_vm_port_link_up_handler[NETACCESS_VM_NUMBER_OF_LINK_UP_HANDLER];
static NETACCESS_VM_PortLinkUpFun_T netaccess_vm_port_link_down_handler[NETACCESS_VM_NUMBER_OF_LINK_DOWN_HANDLER];

static NETACCESS_VM_PortMode_T netaccess_vm_port_mode_ar[NETACCESS_PORTMODE_MAX-1] =
    {
       /*is_allow_add_secure_address*/
         /*is_allow_exec_intrusion_action*/
           /*is_allow_update_mac_entry*/
             /*is_allow_update_mac_entry*/
               /*is_allow_kickout_dot1x_address*/
        {0,0,0,0,0},/* noRestriction */
        {1,1,0,0,0},/* continuousLearning */
        {1,1,0,0,0},/* authLearn */
        {0,1,0,0,0},/* secure */
        {1,0,1,0,0},/* userLogin */
        {1,1,1,0,0},/* userLoginSecure */
        {0,0,0,0,0},/* userLoginWithOUI */
        {1,1,1,0,0},/* macAddressWithRadius */
        {1,1,1,0,0},/* macAddressOrUserLoginSecure */
        {1,1,1,0,0},/* macAddressElseUserLoginSecure */
        {1,0,0,0,0},/* macAuthentication */
        {1,1,0,0,0},/* portSecurity */
        {1,0,0,0,0},/* dot1x */
    };
static NETACCESS_VM_PortMove_T port_move_ar[NETACCESS_PORTMODE_MAX-1][NETACCESS_PORTMODE_MAX-1] =
    {
        {
            /* s2s, s2d, d2s, d2d */
            {TRUE, FALSE, TRUE, TRUE}, /* noRestriction 2 noRestriction */
            {TRUE, FALSE, TRUE, TRUE}, /* noRestriction 2 continuousLearning */
            {TRUE, FALSE, TRUE, TRUE}, /* noRestriction 2 authLearn */
            {TRUE, FALSE, TRUE, TRUE}, /* noRestriction 2 secure */
            {TRUE, FALSE, TRUE, TRUE}, /* noRestriction 2 userLogin */
            {TRUE, FALSE, TRUE, TRUE}, /* noRestriction 2 userLoginSecure */
            {FALSE, FALSE, FALSE, FALSE}, /* noRestriction 2 userLoginWithOUI */
            {TRUE, FALSE, TRUE, TRUE}, /* noRestriction 2 macAddressWithRadius */
            {TRUE, FALSE, TRUE, TRUE}, /* noRestriction 2 macAddressOrUserLoginSecure */
            {TRUE, FALSE, TRUE, TRUE}, /* noRestriction 2 macAddressElseUserLoginSecure */
            {TRUE, FALSE, TRUE, TRUE}, /* noRestriction 2 macAuthentication */
            {TRUE, FALSE, TRUE, TRUE}, /* noRestriction 2 portSecurity */
            {TRUE, FALSE, TRUE, TRUE}, /* noRestriction 2 dot1x */
        },
        {
            /* s2s, s2d, d2s, d2d */
            {FALSE, FALSE, FALSE, FALSE}, /* continuousLearning 2 noRestriction */
            {TRUE, FALSE, TRUE, TRUE}, /* continuousLearning 2 continuousLearning */
            {TRUE, FALSE, TRUE, TRUE}, /* continuousLearning 2 authLearn */
            {TRUE, FALSE, TRUE, TRUE}, /* continuousLearning 2 secure */
            {TRUE, FALSE, TRUE, TRUE}, /* continuousLearning 2 userLogin */
            {TRUE, FALSE, TRUE, TRUE}, /* continuousLearning 2 userLoginSecure */
            {FALSE, FALSE, FALSE, FALSE}, /* continuousLearning 2 userLoginWithOUI */
            {TRUE, FALSE, TRUE, TRUE}, /* continuousLearning 2 macAddressWithRadius */
            {TRUE, FALSE, TRUE, TRUE}, /* continuousLearning 2 macAddressOrUserLoginSecure */
            {TRUE, FALSE, TRUE, TRUE}, /* continuousLearning 2 macAddressElseUserLoginSecure */
            {TRUE, FALSE, TRUE, TRUE}, /* continuousLearning 2 macAuthentication */
            {TRUE, FALSE, TRUE, TRUE}, /* continuousLearning 2 portSecurity */
            {TRUE, FALSE, TRUE, TRUE}, /* continuousLearning 2 dot1x */
        },
        {
            /* s2s, s2d, d2s, d2d */
            {FALSE, FALSE, FALSE, FALSE}, /* authLearn 2 noRestriction */
            {TRUE, FALSE, TRUE, TRUE}, /* authLearn 2 continuousLearning */
            {TRUE, FALSE, TRUE, TRUE}, /* authLearn 2 authLearn */
            {TRUE, FALSE, TRUE, TRUE}, /* authLearn 2 secure */
            {TRUE, FALSE, TRUE, TRUE}, /* authLearn 2 userLogin */
            {TRUE, FALSE, TRUE, TRUE}, /* authLearn 2 userLoginSecure */
            {FALSE, FALSE, FALSE, FALSE}, /* authLearn 2 userLoginWithOUI */
            {TRUE, FALSE, TRUE, TRUE}, /* authLearn 2 macAddressWithRadius */
            {TRUE, FALSE, TRUE, TRUE}, /* authLearn 2 macAddressOrUserLoginSecure */
            {TRUE, FALSE, TRUE, TRUE}, /* authLearn 2 macAddressElseUserLoginSecure */
            {TRUE, FALSE, TRUE, TRUE}, /* authLearn 2 macAuthentication */
            {TRUE, FALSE, TRUE, TRUE}, /* authLearn 2 portSecurity */
            {TRUE, FALSE, TRUE, TRUE}, /* authLearn 2 dot1x */
        },
        {
            /* s2s, s2d, d2s, d2d */
            {FALSE, FALSE, FALSE, FALSE}, /* secure 2 noRestriction */
            {TRUE, FALSE, TRUE, TRUE}, /* secure 2 continuousLearning */
            {TRUE, FALSE, TRUE, TRUE}, /* secure 2 authLearn */
            {TRUE, FALSE, TRUE, TRUE}, /* secure 2 secure */
            {TRUE, FALSE, TRUE, TRUE}, /* secure 2 userLogin */
            {TRUE, FALSE, TRUE, TRUE}, /* secure 2 userLoginSecure */
            {FALSE, FALSE, FALSE, FALSE}, /* secure 2 userLoginWithOUI */
            {TRUE, FALSE, TRUE, TRUE}, /* secure 2 macAddressWithRadius */
            {TRUE, FALSE, TRUE, TRUE}, /* secure 2 macAddressOrUserLoginSecure */
            {TRUE, FALSE, TRUE, TRUE}, /* secure 2 macAddressElseUserLoginSecure */
            {TRUE, FALSE, TRUE, TRUE}, /* secure 2 macAuthentication */
            {TRUE, FALSE, TRUE, TRUE}, /* secure 2 portSecurity */
            {TRUE, FALSE, TRUE, TRUE}, /* secure 2 dot1x */
        },
        {
            /* s2s, s2d, d2s, d2d */
            {FALSE, FALSE, FALSE, FALSE}, /* userLogin 2 noRestriction */
            {TRUE, FALSE, TRUE, TRUE}, /* userLogin 2 continuousLearning */
            {TRUE, FALSE, TRUE, TRUE}, /* userLogin 2 authLearn */
            {TRUE, FALSE, TRUE, TRUE}, /* userLogin 2 secure */
            {TRUE, FALSE, TRUE, TRUE}, /* userLogin 2 userLogin */
            {TRUE, FALSE, TRUE, TRUE}, /* userLogin 2 userLoginSecure */
            {FALSE, FALSE, FALSE, FALSE}, /* userLogin 2 userLoginWithOUI */
            {TRUE, FALSE, TRUE, TRUE}, /* userLogin 2 macAddressWithRadius */
            {TRUE, FALSE, TRUE, TRUE}, /* userLogin 2 macAddressOrUserLoginSecure */
            {TRUE, FALSE, TRUE, TRUE}, /* userLogin 2 macAddressElseUserLoginSecure */
            {TRUE, FALSE, TRUE, TRUE}, /* userLogin 2 macAuthentication */
            {TRUE, FALSE, TRUE, TRUE}, /* userLogin 2 portSecurity */
            {TRUE, FALSE, TRUE, TRUE}, /* userLogin 2 dot1x */
        },
        {
            /* s2s, s2d, d2s, d2d */
            {FALSE, FALSE, FALSE, FALSE}, /* userLoginSecure 2 noRestriction */
            {TRUE, FALSE, TRUE, TRUE}, /* userLoginSecure 2 continuousLearning */
            {TRUE, FALSE, TRUE, TRUE}, /* userLoginSecure 2 authLearn */
            {TRUE, FALSE, TRUE, TRUE}, /* userLoginSecure 2 secure */
            {TRUE, FALSE, TRUE, TRUE}, /* userLoginSecure 2 userLogin */
            {TRUE, FALSE, TRUE, TRUE}, /* userLoginSecure 2 userLoginSecure */
            {FALSE, FALSE, FALSE, FALSE}, /* userLoginSecure 2 userLoginWithOUI */
            {TRUE, FALSE, TRUE, TRUE}, /* userLoginSecure 2 macAddressWithRadius */
            {TRUE, FALSE, TRUE, TRUE}, /* userLoginSecure 2 macAddressOrUserLoginSecure */
            {TRUE, FALSE, TRUE, TRUE}, /* userLoginSecure 2 macAddressElseUserLoginSecure */
            {TRUE, FALSE, TRUE, TRUE}, /* userLoginSecure 2 macAuthentication */
            {TRUE, FALSE, TRUE, TRUE}, /* userLoginSecure 2 portSecurity */
            {TRUE, FALSE, TRUE, TRUE}, /* userLoginSecure 2 dot1x */
        },
        {
            /* s2s, s2d, d2s, d2d */
            {FALSE, FALSE, FALSE, FALSE}, /* userLoginWithOUI 2 noRestriction */
            {TRUE, FALSE, TRUE, TRUE}, /* userLoginWithOUI 2 continuousLearning */
            {TRUE, FALSE, TRUE, TRUE}, /* userLoginWithOUI 2 authLearn */
            {TRUE, FALSE, TRUE, TRUE}, /* userLoginWithOUI 2 secure */
            {TRUE, FALSE, TRUE, TRUE}, /* userLoginWithOUI 2 userLogin */
            {TRUE, FALSE, TRUE, TRUE}, /* userLoginWithOUI 2 userLoginSecure */
            {TRUE, TRUE, TRUE, TRUE}, /* userLoginWithOUI 2 userLoginWithOUI */
            {TRUE, FALSE, TRUE, TRUE}, /* userLoginWithOUI 2 macAddressWithRadius */
            {TRUE, FALSE, TRUE, TRUE}, /* userLoginWithOUI 2 macAddressOrUserLoginSecure */
            {TRUE, FALSE, TRUE, TRUE}, /* userLoginWithOUI 2 macAddressElseUserLoginSecure */
            {TRUE, FALSE, TRUE, TRUE}, /* userLoginWithOUI 2 macAuthentication */
            {TRUE, FALSE, TRUE, TRUE}, /* userLoginWithOUI 2 portSecurity */
            {TRUE, FALSE, TRUE, TRUE}, /* userLoginWithOUI 2 dot1x */
        },
        {
            /* s2s, s2d, d2s, d2d */
            {FALSE, FALSE, FALSE, FALSE}, /* macAddressWithRadius 2 noRestriction */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressWithRadius 2 continuousLearning */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressWithRadius 2 authLearn */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressWithRadius 2 secure */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressWithRadius 2 userLogin */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressWithRadius 2 userLoginSecure */
            {FALSE, FALSE, FALSE, FALSE}, /* macAddressWithRadius 2 userLoginWithOUI */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressWithRadius 2 macAddressWithRadius */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressWithRadius 2 macAddressOrUserLoginSecure */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressWithRadius 2 macAddressElseUserLoginSecure */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressWithRadius 2 macAuthentication */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressWithRadius 2 portSecurity */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressWithRadius 2 dot1x */
        },
        {
            /* s2s, s2d, d2s, d2d */
            {FALSE, FALSE, FALSE, FALSE}, /* macAddressOrUserLoginSecure 2 noRestriction */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressOrUserLoginSecure 2 continuousLearning */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressOrUserLoginSecure 2 authLearn */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressOrUserLoginSecure 2 secure */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressOrUserLoginSecure 2 userLogin */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressOrUserLoginSecure 2 userLoginSecure */
            {FALSE, FALSE, FALSE, FALSE}, /* macAddressOrUserLoginSecure 2 userLoginWithOUI */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressOrUserLoginSecure 2 macAddressWithRadius */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressOrUserLoginSecure 2 macAddressOrUserLoginSecure */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressOrUserLoginSecure 2 macAddressElseUserLoginSecure */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressOrUserLoginSecure 2 macAuthentication */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressOrUserLoginSecure 2 portSecurity */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressOrUserLoginSecure 2 dot1x */
        },
        {
            /* s2s, s2d, d2s, d2d */
            {FALSE, FALSE, FALSE, FALSE}, /* macAddressElseUserLoginSecure 2 noRestriction */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressElseUserLoginSecure 2 continuousLearning */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressElseUserLoginSecure 2 authLearn */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressElseUserLoginSecure 2 secure */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressElseUserLoginSecure 2 userLogin */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressElseUserLoginSecure 2 userLoginSecure */
            {FALSE, FALSE, FALSE, FALSE}, /* macAddressElseUserLoginSecure 2 userLoginWithOUI */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressElseUserLoginSecure 2 macAddressWithRadius */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressElseUserLoginSecure 2 macAddressOrUserLoginSecure */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressElseUserLoginSecure 2 macAddressElseUserLoginSecure */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressElseUserLoginSecure 2 macAuthentication */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressElseUserLoginSecure 2 portSecurity */
            {TRUE, FALSE, TRUE, TRUE}, /* macAddressElseUserLoginSecure 2 dot1x */
        },
        {
            /* s2s, s2d, d2s, d2d */
            {TRUE, TRUE, TRUE, TRUE}, /* macAuthentication 2 noRestriction */
            {TRUE, FALSE, TRUE, TRUE}, /* macAuthentication 2 continuousLearning */
            {TRUE, FALSE, TRUE, TRUE}, /* macAuthentication 2 authLearn */
            {TRUE, FALSE, TRUE, TRUE}, /* macAuthentication 2 secure */
            {TRUE, FALSE, TRUE, TRUE}, /* macAuthentication 2 userLogin */
            {TRUE, FALSE, TRUE, TRUE}, /* macAuthentication 2 userLoginSecure */
            {FALSE, FALSE, FALSE, FALSE}, /* macAuthentication 2 userLoginWithOUI */
            {TRUE, FALSE, TRUE, TRUE}, /* macAuthentication 2 macAddressWithRadius */
            {TRUE, FALSE, TRUE, TRUE}, /* macAuthentication 2 macAddressOrUserLoginSecure */
            {TRUE, FALSE, TRUE, TRUE}, /* macAuthentication 2 macAddressElseUserLoginSecure */
            {TRUE, TRUE, TRUE, TRUE}, /* macAuthentication 2 macAuthentication */
            {TRUE, TRUE, TRUE, TRUE}, /* macAuthentication 2 portSecurity */
            {TRUE, TRUE, TRUE, TRUE}, /* macAuthentication 2 dot1x */
        },
        {
            /* s2s, s2d, d2s, d2d */
            {TRUE, FALSE, FALSE, TRUE}, /* portSecurity 2 noRestriction */
            {TRUE, FALSE, TRUE, TRUE}, /* portSecurity 2 continuousLearning */
            {TRUE, FALSE, TRUE, TRUE}, /* portSecurity 2 authLearn */
            {TRUE, FALSE, TRUE, TRUE}, /* portSecurity 2 secure */
            {TRUE, FALSE, TRUE, TRUE}, /* portSecurity 2 userLogin */
            {TRUE, FALSE, TRUE, TRUE}, /* portSecurity 2 userLoginSecure */
            {FALSE, FALSE, FALSE, FALSE}, /* portSecurity 2 userLoginWithOUI */
            {TRUE, FALSE, TRUE, TRUE}, /* portSecurity 2 macAddressWithRadius */
            {TRUE, FALSE, TRUE, TRUE}, /* portSecurity 2 macAddressOrUserLoginSecure */
            {TRUE, FALSE, TRUE, TRUE}, /* portSecurity 2 macAddressElseUserLoginSecure */
            {TRUE, FALSE, TRUE, TRUE}, /* portSecurity 2 macAuthentication */
            {TRUE, FALSE, TRUE, TRUE}, /* portSecurity 2 portSecurity */
            {TRUE, FALSE, TRUE, TRUE}, /* portSecurity 2 dot1x */
        },
        {
            /* s2s, s2d, d2s, d2d */
            {TRUE, FALSE, FALSE, TRUE}, /* dot1x 2 noRestriction */
            {TRUE, FALSE, TRUE, TRUE}, /* dot1x 2 continuousLearning */
            {TRUE, FALSE, TRUE, TRUE}, /* dot1x 2 authLearn */
            {TRUE, FALSE, TRUE, TRUE}, /* dot1x 2 secure */
            {TRUE, FALSE, TRUE, TRUE}, /* dot1x 2 userLogin */
            {TRUE, FALSE, TRUE, TRUE}, /* dot1x 2 userLoginSecure */
            {FALSE, FALSE, FALSE, FALSE}, /* dot1x 2 userLoginWithOUI */
            {TRUE, FALSE, TRUE, TRUE}, /* dot1x 2 macAddressWithRadius */
            {TRUE, FALSE, TRUE, TRUE}, /* dot1x 2 macAddressOrUserLoginSecure */
            {TRUE, FALSE, TRUE, TRUE}, /* dot1x 2 macAddressElseUserLoginSecure */
            {TRUE, FALSE, TRUE, TRUE}, /* dot1x 2 macAuthentication */
            {TRUE, FALSE, TRUE, TRUE}, /* dot1x 2 portSecurity */
            {TRUE, FALSE, TRUE, TRUE}, /* dot1x 2 dot1x */
        },
    };

/* EXPORTED SUBPROGRAM BODIES
 */
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_Initialize
 * ---------------------------------------------------------------------
 * PURPOSE: initialize VM
 * INPUT:  none
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_Initialize()
{
    /* initial function pointer
     */
    NETACCESS_VM_LocalInitializeFunctionPointer();

    /* initial VM
     */
    return NETACCESS_VM_LocalInitializeVM();
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessEventPortModeChange
 * ---------------------------------------------------------------------
 * PURPOSE: process event (port mode change)
 * INPUT:  lport, new_port_mode
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessEventPortModeChange(UI32_T lport, NETACCESS_PortMode_T new_port_mode)
{
    NETACCESS_OM_StateMachine_T     state_machine;

    /* get state machine from database
     */
    if (FALSE == NETACCESS_OM_GetPortStateMachine(lport, &state_machine))
    {
        return FALSE;
    }

    /* check if anything change
     */
    if (state_machine.running_port_mode == new_port_mode)
    {
        return TRUE;
    }

    /* set new port mode to state machine
     */
    state_machine.new_port_mode = new_port_mode;

    /* run state machine
     */
    NETACCESS_VM_RUN_PORT_MODE_CHANGE_SM(lport, &state_machine);

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessEventNewMac
 * ---------------------------------------------------------------------
 * PURPOSE: process event (new mac)
 * INPUT:  msg
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessEventNewMac(NETACCESS_NEWMAC_MSGQ_T *new_mac_msg_ptr)
{
    UI32_T                          debug_flag, lport;
    NETACCESS_NEWMAC_DATA_T         *newmac_data;
    NETACCESS_EAP_DATA_T            *eap_data;
    NETACCESS_OM_StateMachine_T     state_machine;
    BOOL_T                          ret;

    /* check if input is valid
     */
    if (NULL == new_mac_msg_ptr)
    {
        return FALSE;
    }

    debug_flag = NETACCESS_OM_GetDebugFlag();

    NETACCESS_VM_DEBUG_PRINT_FUNCTION_START();

    /* check what kind of callback
     */
    newmac_data = new_mac_msg_ptr->m_newmac_data;
    eap_data    = new_mac_msg_ptr->m_eap_data;

    if (NULL != newmac_data)
    {
        /* from new mac callback
         */
        lport = newmac_data->lport;
    }
    else if (NULL != eap_data)
    {
        /* from eap callback
         */
        lport = eap_data->lport_no;
    }
    else
    {
        /* msg has bad content
         */
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] msg has bad content", __FUNCTION__);
        return FALSE;
    }

    /* don't process this event if port down
     */
    if (FALSE == NETACCESS_VM_LocalIsPortOperUp(lport))
    {
        if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
            printf("\r\n[%s] not oper-up port(%lu)", __FUNCTION__, lport);
        return FALSE;
    }

    /* get state machine
     */
    if (FALSE == NETACCESS_OM_GetPortStateMachine(lport, &state_machine))
    {
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] NETACCESS_OM_GetPortStateMachine(%lu) failed", __FUNCTION__, lport);
        return FALSE;
    }

    /* don't process event if this mac has been unauthenticated
     */
    if(TRUE == NETACCESS_OM_IsMacExistInUnauthorizedMacCache((NULL != newmac_data)?
                                                             newmac_data->src_mac : eap_data->src_mac))
    {
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] unauthenticated MAC ", __FUNCTION__);
        return FALSE;
    }

    /* trigger event
     */
    if (NULL != newmac_data)
    {
        /* if inject a burst stream, it is possible that more than one new mac are enqueued.
         * as a result, check is_authenticating flag to avoid second new mac to disturb
         * authenticating process of the first one.
         */
        if (1 == state_machine.port_security_sm.event_bitmap.is_authenticating)
        {
            if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
            {
                printf("\r\n[%s] lport(%lu) is authenticating... drop mac:", __FUNCTION__, lport);
                NETACCESS_VM_LocalPrintMacAddr(newmac_data->src_mac);
            }
            return FALSE;
        }

#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)

        /* port move has high priority
         */
        if (L2MUX_MGR_RECV_REASON_STATION_MOVE & newmac_data->reason)
        {
            if (FALSE == NETACCESS_VM_ProcessEventPortMove(lport, newmac_data->src_mac, newmac_data->vid))
            {
                if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
                    printf("\r\n[%s] station move,process port move fail", __FUNCTION__);
                return FALSE;
            }
        }
        else if (L2MUX_MGR_RECV_REASON_INTRUDER & newmac_data->reason)
        {
            /* detect port move issue from hisam
             * unauthorized mac didn't write to chip, so it must be detected by software
             */
            if (FALSE == NETACCESS_VM_DetectPortMove(lport, newmac_data->src_mac, newmac_data->vid))
            {
                if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
                    printf("\r\n[%s] intrusion,detect port move fail", __FUNCTION__);
                return FALSE;
            }
        }
        else
        {
            /* unknown reason
             */
            if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
                printf("\r\n[%s] unknown reason", __FUNCTION__);
            return FALSE;
        }
#endif /* #if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE) */

        /* check by voice VLAN if ingress VID equals VVID
         */
        if (   (TRUE == NETACCESS_VM_LocalIsVoiceVlanEnabled(lport))
            && (TRUE == NETACCESS_VM_LocalIsVoiceVlanId(newmac_data->vid))
           )
        {
            NETACCESS_VM_LocalByPassToVoiceVlan(newmac_data->vid, newmac_data->src_mac, lport);

            if (newmac_data->cookie)
            {
                SYS_CALLBACK_MGR_AuthenticatePacket_AsyncReturn(SYS_MODULE_NETACCESS, SYS_CALLBACK_MGR_AUTH_UNAUTHENTICATED, newmac_data->cookie);
                newmac_data->cookie = NULL;
            }

            return TRUE;
        }

        state_machine.port_security_sm.event_bitmap.new_mac = 1;
        memcpy(state_machine.port_security_sm.authenticating_mac, newmac_data->src_mac, SYS_ADPT_MAC_ADDR_LEN);
        state_machine.port_security_sm.src_vid = newmac_data->vid;
        state_machine.port_security_sm.event_bitmap.is_tagged = (TRUE == newmac_data->is_tag_packet) ? 1 : 0;

        /* clear the old cookie
         */
        if (NULL != state_machine.port_security_sm.cookie)
        {
            NETACCESS_VM_AuthResult_T auth_result;

            auth_result.result = SYS_CALLBACK_MGR_AUTH_UNAUTHENTICATED;
            auth_result.vid = 0;

            NETACCESS_VM_LocalPassCookieToNextCSC(lport,
                                                  &state_machine,
                                                  &auth_result);
        }
        state_machine.port_security_sm.cookie = newmac_data->cookie;
        newmac_data->cookie = NULL;

        /* port security mode should not check filter table
         */
        if(NETACCESS_PORTMODE_PORT_SECURITY != state_machine.running_port_mode)
        {
            /* check if this MAC is pre-authenticated
             */
            if (TRUE == NETACCESS_VM_IsMacFilterMatched(lport, newmac_data->src_mac))
            {
                return NETACCESS_VM_LocalLearnPreauthenticatedMac(lport, &state_machine);
            }
        }

        /* If the MAC is authorized(exist in secure address table), pass all traffic.
         * This check should be done for new MAC only.
         */
        {
            NETACCESS_OM_SecureKey_T        key;

            memset(&key, 0, sizeof(key));
            key.lport = lport;
            memcpy(key.secure_mac, state_machine.port_security_sm.authenticating_mac, SYS_ADPT_MAC_ADDR_LEN);
            if (TRUE == NETACCESS_OM_DoesRecordExistInHisamBySecureKey(&key))
            {
                return NETACCESS_VM_LocalAddAuthenticatingMacToAmtr(lport, &state_machine);
            }
        }
    }
    else
    {
        /* if inject a burst stream, it is possible that more than one EAP with different src_mac are enqueued.
         * as a result, check is_authenticating flag and src_mac to avoid second EAP with different src_mac
         * to disturb authenticating process of the first one.
         */
        if (1 == state_machine.port_security_sm.event_bitmap.is_authenticating)
        {
            /* EPR ES4649-32-01139
             */
            if (FALSE == NETACCESS_OM_IsThisMacAuthenticatingMac(lport, eap_data->src_mac))
            {
                if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
                {
                    printf("\r\n[%s] lport(%lu) drop EAP packet because wrong mac:", __FUNCTION__, lport);
                    NETACCESS_VM_LocalPrintMacAddr(eap_data->src_mac);
                }
                return FALSE;
            }
        }
        else
        {
            memcpy(state_machine.port_security_sm.authenticating_mac, eap_data->src_mac, SYS_ADPT_MAC_ADDR_LEN);
        }

        state_machine.port_security_sm.event_bitmap.eap_packet = 1;

        /* clear the old cookie
         */
        if (NULL != state_machine.port_security_sm.cookie)
        {
            NETACCESS_VM_AuthResult_T auth_result;

            auth_result.result = SYS_CALLBACK_MGR_AUTH_UNAUTHENTICATED;
            auth_result.vid = 0;

            NETACCESS_VM_LocalPassCookieToNextCSC(lport,
                                                  &state_machine,
                                                  &auth_result);
        }

        state_machine.port_security_sm.cookie = eap_data->cookie;
        eap_data->cookie = NULL;

        if (TRUE == NETACCESS_OM_IsThisMacAuthorizedByDot1x(lport, state_machine.port_security_sm.authenticating_mac))
        {
            state_machine.port_security_sm.event_bitmap.reauth = 1;
        }
    }

    /* duplicate pointer to om
     */
    state_machine.port_security_sm.new_mac_msg = new_mac_msg_ptr;

    if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
        printf("\r\n[%s] port %ld,run state machine", __FUNCTION__, lport);

    /* run state machine
     */
    ret = NETACCESS_VM_RunSecurePortModeSM(lport, &state_machine);

    /* run state machine fail action
     */
    if (FALSE == ret)
    {
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] NETACCESS_VM_RunSecurePortModeSM(%lu) failed", __FUNCTION__, lport);

        if (NULL != state_machine.port_security_sm.cookie)
        {
            NETACCESS_VM_AuthResult_T auth_result;

            auth_result.result = SYS_CALLBACK_MGR_AUTH_UNAUTHENTICATED;
            auth_result.vid = 0;

            NETACCESS_VM_LocalPassCookieToNextCSC(lport,
                                                  &state_machine,
                                                  &auth_result);
        }

        if (FALSE == NETACCESS_OM_StopStateMachineDoAuthentication(lport)) /* to avoid state machine to be checkmated */
        {
        }
        NETACCESS_OM_DestroyAllPortEapData(lport);
    }

    /* must clean up new_mac_msg no matter ret is TURE or FALSE
     */
    if (FALSE == NETACCESS_OM_ClearStateMachineNewMacMsg(lport))
    {
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] NETACCESS_OM_ClearStateMachineNewMacMsg(%lu) failed", __FUNCTION__, lport);
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_ProcessEventDelAcl
 *-------------------------------------------------------------------------
 * PURPOSE  : Process for detecting a ACL be deleted.
 * INPUT    : acl_name          -- which ACL be deleted.
 *            acl_type          -- which type of ACL
 *            dynamic_port_list -- the port list that bind the deleted ACL
 *                                 with dynamic type.
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void NETACCESS_VM_ProcessEventDelAcl(const char *acl_name, UI32_T acl_type, UI8_T *dynamic_port_list)
{
    UI32_T index;

    /* When a policy map be deleted, it will cause all users logoff who use it.
     */
    for (index = 0; index < SYS_ADPT_TOTAL_NBR_OF_LPORT; ++index)
    {
        if ( !(dynamic_port_list[index/8] & (1 << (7 - index%8))) )
            continue;

        /* Only one policy map/acl can be bound on a port with dynamic type.
         * So that just need to check the flag to know that those ports
         * shall be effected.
         * E.g.,
         *           | port 1 | port 2
         *       ----+--------+--------
         *        p1 | static | dynamic
         *       ----+--------+--------
         *        p2 | dynamic|  ---
         *       ----+--------+--------
         * These have two policy map be created p1 and p2.
         * Port 1 and port 2 have already bound dynamic policy map.
         * When p1 be deleted, the port list shall include port 2 only.
         * Then delete all authorized user from port 2.
         */
        NETACCESS_VM_DeleteAllAuthorizedUserByPort(index + 1);

#if (SYS_CPNT_DOT1X == TRUE)
        DOT1X_VM_SendEvent(index + 1, DOT1X_SM_AUTH_EAPLOGOFF_EV);
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_ProcessEventDelPolicyMap
 *-------------------------------------------------------------------------
 * PURPOSE  : Process for detecting a policy nao be deleted.
 * INPUT    : policy_map_name       -- which acl be deleted.
 *            dynamic_port_list     -- the port list that bind the deleted acl
 *                                     with dynamic type.
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void NETACCESS_VM_ProcessEventDelPolicyMap(const char *policy_map_name, UI8_T *dynamic_port_list)
{
    UI32_T index;

    /* When a policy map be deleted, it will cause all users logoff who use it.
     */
    for (index = 0; index < SYS_ADPT_TOTAL_NBR_OF_LPORT; ++index)
    {
        if ( !(dynamic_port_list[index/8] & (1 << (7 - index%8))) )
            continue;

        /* Only one policy map/acl can be bound on a port with dynamic type.
         * So that just need to check the flag to know that those ports
         * shall be effected.
         * E.g.,
         *           | port 1 | port 2
         *       ----+--------+--------
         *        p1 | static | dynamic
         *       ----+--------+--------
         *        p2 | dynamic|  ---
         *       ----+--------+--------
         * These have two policy map be created p1 and p2.
         * Port 1 and port 2 have already bound dynamic policy map.
         * When p1 be deleted, the port list shall include port 2 only.
         * Then delete all authorized user from port 2.
         */
        NETACCESS_VM_DeleteAllAuthorizedUserByPort(index + 1);

#if (SYS_CPNT_DOT1X == TRUE)
        DOT1X_VM_SendEvent(index + 1, DOT1X_SM_AUTH_EAPLOGOFF_EV);
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */
    }
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessEventPortMove
 * ---------------------------------------------------------------------
 * PURPOSE: process event (port move)
 * INPUT:  lport, new_mac, vid
 * OUTPUT: None.
 * RETURN: TRUE -- should learn the new mac / FALSE -- should not learn this new mac
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessEventPortMove(UI32_T lport, UI8_T *new_mac, UI32_T vid)
{
    UI32_T debug_flag = 0;
    UI32_T new_port_mode, original_lport, original_port_mode;
    BOOL_T is_from_static = FALSE;

    NETACCESS_OM_MacAdrKey_T        key;
    NETACCESS_OM_HISAMentry_T       hisam_entry;

    /* get running port mode
     */
    if (FALSE == NETACCESS_OM_GetPortStateMachineRunningPortMode(lport, &new_port_mode))
    {
        return FALSE;
    }

    debug_flag = NETACCESS_OM_GetDebugFlag();

    if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
        printf("\r\n[%s] start processing...", __FUNCTION__);

    /* search which port having new_mac
     */
    memset(&key, 0, sizeof(NETACCESS_OM_MacAdrKey_T));
    memcpy(key.secure_mac, new_mac, SYS_ADPT_MAC_ADDR_LEN);

    /* get next HISAM record
     */
    if ((TRUE == NETACCESS_OM_GetNextHisamRecordByMacKey(&key, &hisam_entry)) &&
        (0 == memcmp(hisam_entry.secure_mac, new_mac, SYS_ADPT_MAC_ADDR_LEN)))
    {
        if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
            printf("\r\n[%s] get secure mac addressentry success", __FUNCTION__);

        /* get original port's mode
         */
        if (FALSE == NETACCESS_OM_GetPortStateMachineRunningPortMode(key.lport, &original_port_mode))
        {
            return FALSE;
        }

        /* check admin_configured_mac,
         * administrative configured mac has HIGHEST priority
         */
        if (TRUE == NETACCESS_OM_IsSecureAddressAdminConfigured(hisam_entry.mac_index))
        {
            if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
                printf("\r\n[%s] can't replace administrative configured mac", __FUNCTION__);
            return FALSE;
        }

        /* check auto port move issue
         */
        if (FALSE == NETACCESS_VM_LocalCheckAutoPortMoveIssue(original_port_mode, new_port_mode, is_from_static, FALSE))
        {
            if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
                printf("\r\n[%s] the address is already in the table for a port(%lu)", __FUNCTION__, key.lport);
            return FALSE;
        }

        /* delete port move MAC address
         */
        if (FALSE == NETACCESS_VM_DeleteAuthorizedUserByIndex(hisam_entry.mac_index))
        {
            return FALSE;
        }
    }
    else
    {
        /* if original port mode is noRestrictions or userLoginSecure,
         * new_mac may be learned by chip, not NETACCESS.
         */
        UI32_T tmp_vid = 0;
        /*AMTR_TYPE_AddrEntry_T addr_entry;*/

        if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
            printf("\r\n[%s] get secure mac addressentry fail", __FUNCTION__);

        /*memset(&addr_entry, 0, sizeof(AMTR_TYPE_AddrEntry_T));*/

        /* get next MAC address
         */
        /*while (TRUE == NETACCESS_VM_LocalGetNextMacFromAmtr(new_mac, &tmp_vid, &addr_entry))*/
        while (TRUE == NETACCESS_VM_LocalGetNextMacFromAmtr(new_mac, &original_lport, &tmp_vid))
        {
            /* mac entry's port should not equal new port
             */
            /*if(lport == addr_entry.ifindex)*/
            if(lport == original_lport)
            {
                continue;
            }

            /* get original port's mode
             */
            if (FALSE == NETACCESS_OM_GetPortStateMachineRunningPortMode(original_lport, &original_port_mode))
            {
                if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
                    printf("\r\n[%s] get running port mode fail for a port(%lu)", __FUNCTION__, original_lport);
                return FALSE;
            }

            /* macAuthentication mode should have secure address entry and will not go here
             */
            if(NETACCESS_PORTMODE_MAC_AUTHENTICATION == original_port_mode)
            {
                continue;
            }

            /* check mac entry attribute
             */
#if RickyModify
            is_from_static = ((addr_entry.attribute == AMTR_MGR_ENTRY_ATTRIBUTE_PERMANENT) ||
                               (addr_entry.attribute == AMTR_MGR_ENTRY_ATTRIBUTE_DELETE_ON_RESET))
                             ? TRUE : FALSE;
#endif

            /* check auto port move issue
             */
            if (FALSE == NETACCESS_VM_LocalCheckAutoPortMoveIssue(original_port_mode, new_port_mode, is_from_static, FALSE))
            {
                if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
                    printf("\r\n[%s] the address is already in the table for a port(%lu)", __FUNCTION__, original_lport);
                return FALSE;
            }

            /* delete MAC address
             */
#if 1 /*RickyModify*/
            if(FALSE == AMTR_MGR_DeleteAddr(tmp_vid, new_mac))
            {
                if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
                    printf("\r\n[%s] call AMTR_MGR_DeleteAddr() failed", __FUNCTION__);
            }
#endif

        }
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessEventPortLinkUp
 * ---------------------------------------------------------------------
 * PURPOSE: process event (port link-up)
 * INPUT:  lport
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessEventPortLinkUp(UI32_T lport)
{
    int i;
    UI32_T  unit, port, trunk_id;

    NETACCESS_VM_DEBUG_PRINT_IMPORTANT("port(%lu) link-up", lport);

#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) &&
         SWCTRL_LPORT_TRUNK_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
#else
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
#endif
    {
        return FALSE;
    }

    for(i=0; i<NETACCESS_VM_NUMBER_OF_LINK_UP_HANDLER; ++i)
    {
        if (netaccess_vm_port_link_up_handler[i])
        {
            netaccess_vm_port_link_up_handler[i](lport);
        }
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessEventPortLinkDown
 * ---------------------------------------------------------------------
 * PURPOSE: process event (port link-down)
 * INPUT:  lport
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessEventPortLinkDown(UI32_T lport)
{
    int i;
    UI32_T  unit, port, trunk_id;

    NETACCESS_VM_DEBUG_PRINT_IMPORTANT("port(%lu) link-down", lport);

    /* check if port is valid
     */
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) &&
         SWCTRL_LPORT_TRUNK_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
#else
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
#endif
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("Wrong lport(%lu)", lport);
        return FALSE;
    }

    for(i=0; i<NETACCESS_VM_NUMBER_OF_LINK_DOWN_HANDLER; ++i)
    {
        if (netaccess_vm_port_link_down_handler[i])
        {
            netaccess_vm_port_link_down_handler[i](lport);
        }
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessEventPortAdminUp
 * ---------------------------------------------------------------------
 * PURPOSE: process event (port admin-up)
 * INPUT:  lport
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessEventPortAdminUp(UI32_T lport)
{
    UI32_T  unit, port, trunk_id;

    NETACCESS_VM_DEBUG_PRINT_TRACE("lport(%lu)", lport);

    /* check if port is valid
     */
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) &&
         SWCTRL_LPORT_TRUNK_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
#else
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
#endif
    {
        return FALSE;
    }

#if (SYS_CPNT_DOT1X == TRUE)
    /* ES4649-32-00999
     * must notify dot1x to run state machine
     */
    DOT1X_VM_NotifyPortAdminUp(unit, port);
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessEventPortAdminDown
 * ---------------------------------------------------------------------
 * PURPOSE: process event (port admin-down)
 * INPUT:  lport
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessEventPortAdminDown(UI32_T lport)
{
    UI32_T  unit, port, trunk_id;

    NETACCESS_VM_DEBUG_PRINT_TRACE("lport(%lu)", lport);

    /* check if port is valid
     */
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) &&
         SWCTRL_LPORT_TRUNK_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
#else
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
#endif
    {
        return FALSE;
    }

#if (SYS_CPNT_DOT1X == TRUE)
    /* ES4649-32-00999
     * must notify dot1x to run state machine
     */
    DOT1X_VM_NotifyPortAdminDown(unit, port);
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    return NETACCESS_VM_LocalPortDown(lport);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessEventPortVlanChange
 * ---------------------------------------------------------------------
 * PURPOSE: process event (port join/depart vlan)
 * INPUT:  vid,lport,status,change_event
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessEventPortVlanChange(UI32_T vid, UI32_T lport, UI32_T status, NETACCESS_VlanModified_T change_event)
{
    UI32_T      debug_flag = 0, unit, port, trunk_id;
    NETACCESS_OM_SecureMacEntry_T  mac_entry;

    //NETACCESS_VM_CHECK_PSEC_CONTROL_MODE(TRUE); /* don't treat it as an error */

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* check if permanent VLAN
     */
    if (VAL_dot1qVlanStatus_permanent != status)
    {
        /* don't care
         */
        return TRUE;
    }

    if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
        printf("\r\n[%s] port(%lu) join vlan(%lu) processing...", __FUNCTION__, lport, vid);

    /* check if port is reasonable
     */
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) &&
         SWCTRL_LPORT_TRUNK_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
#else
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
#endif
    {
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] bad lport(%lu)", __FUNCTION__, lport);
        return FALSE;
    }

    /* check if port allow vlan change
     */
    if(FALSE == NETACCESS_VM_LocalIsPortAllowVlanChange(lport))
    {
        /* don't care
         */
        return TRUE;
    }

    /* prepare searching key
     */
    memset(&mac_entry, 0, sizeof(mac_entry));
    mac_entry.lport = lport;

    /* add/delete every mac on lport with vid
     */
    while (TRUE == NETACCESS_OM_GetNextSecureAddressEntry(&mac_entry))
    {
        /* check if same lport
         */
        if (lport != mac_entry.lport)
        {
            break;
        }

        /* check if ever write to AMTR
         */
        if (0 == mac_entry.mac_flag.write_to_amtr)
        {
            continue;
        }

        if(NETACCESS_PORT_ADDED == change_event)
        {
            /* set MAC address to AMTR
             */
        }
        else
        {
            /* delete MAC address
             */
            if (FALSE == AMTR_MGR_DeleteAddr(vid, mac_entry.secure_mac))
            {
                if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
                    printf("\r\n[%s] AMTR_MGR_SetPortSecurityAddrEntry() failed", __FUNCTION__);
                /* continue even if fail
                 */
            }
            else
            {
                /* update counter
                 */
                NETACCESS_OM_IncAmtrDeleteAddrCounter();
            }
        }
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessEventReauthentication
 * ---------------------------------------------------------------------
 * PURPOSE: process event (reauthenticate mac)
 * INPUT:  reauth_mac
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessEventReauthentication(UI8_T *reauth_mac)
{
    UI32_T                      debug_flag, unit, port, trunk_id;
    NETACCESS_OM_MacAdrKey_T    key;
    NETACCESS_OM_HISAMentry_T   hisam_entry;
    NETACCESS_OM_StateMachine_T state_machine;

    debug_flag = NETACCESS_OM_GetDebugFlag();

    if (NETACCESS_OM_DEBUG_VM_IFO & debug_flag)
    {
        printf("\r\n[%s] reauthenticate mac:", __FUNCTION__);
        NETACCESS_VM_LocalPrintMacAddr(reauth_mac);
    }

    /* search which port having reauth_mac
     */
    memset(&key, 0, sizeof(NETACCESS_OM_MacAdrKey_T));
    memcpy(key.secure_mac, reauth_mac, SYS_ADPT_MAC_ADDR_LEN);

    /* get next HISAM record
     */
    if ((FALSE == NETACCESS_OM_GetNextHisamRecordByMacKey(&key, &hisam_entry)) ||
        (0 != memcmp(hisam_entry.secure_mac, reauth_mac, SYS_ADPT_MAC_ADDR_LEN)))
    {
        if (NETACCESS_OM_DEBUG_VM_RST & debug_flag)
        {
            printf("\r\n[%s] in hisam, can't find mac:", __FUNCTION__);
            NETACCESS_VM_LocalPrintMacAddr(reauth_mac);
        }

        /* A3COM0458 Mib, secureRadaReauthenticate
         * If the MAC address not currently known to RADA,
         * it silently ignores the write.
         * ==> so return TRUE here
         */
        return TRUE;
    }

    /* get state machine
     */
    if ((SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(key.lport, &unit, &port, &trunk_id)) ||
        (FALSE == NETACCESS_OM_GetPortStateMachine(key.lport, &state_machine)))
    {
        return FALSE;
    }

    /* check if port is authenticating
     */
    if (1 == state_machine.port_security_sm.event_bitmap.is_authenticating)
    {
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] port(%lu) is authenticating... can't do reauth", __FUNCTION__, key.lport);
        return FALSE;
    }

    /* check if allow do reauthentication
     */
    if(FALSE == NETACCESS_VM_LocalIsSmAllowAddressReauth(&state_machine, hisam_entry.mac_index))
    {
        return TRUE;
    }

    /* trigger event
     */
    state_machine.port_security_sm.event_bitmap.reauth = 1;
    memcpy(state_machine.port_security_sm.authenticating_mac, reauth_mac, SYS_ADPT_MAC_ADDR_LEN);

    /* run state machine
     */
    return NETACCESS_VM_RunSecurePortModeSM(key.lport, &state_machine);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessEventTimerUp
 * ---------------------------------------------------------------------
 * PURPOSE: process event (timer period)
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessEventTimerUp()
{
    UI32_T                      unit, port, trunk_id, intrusion_action, expire_time;
    UI32_T                      lport, sys_time, debug_flag, unauth_expire_time;
    NETACCESS_OM_ExpireKey_T        key;
    NETACCESS_OM_StateMachine_T     state_machine;
    NETACCESS_OM_HISAMentry_T       hisam_entry;

    debug_flag = NETACCESS_OM_GetDebugFlag();

    SYS_TIME_GetRealTimeBySec(&sys_time);

    memset(&key, 0, sizeof(NETACCESS_OM_ExpireKey_T));

    /* check if time expire for unauthenticated mac table
     */
    if((TRUE == NETACCESS_OM_GetUnauthorizedMacCacheExpireTime(&unauth_expire_time)) &&
        (sys_time > unauth_expire_time))
    {
        UI32_T holdoff_time;

        NETACCESS_OM_ClearUnauthorizedMacCache();

        NETACCESS_OM_GetSecureHoldoffTime(&holdoff_time);
        NETACCESS_OM_SetUnauthorizedMacCacheExpireTime(sys_time + holdoff_time);
    }

    /* get next time up authenticated MAC address entry
     */
    while (TRUE == NETACCESS_OM_GetNextHisamRecordByExpireKey(&key, &hisam_entry))
    {
        NETACCESS_OM_SecureMacEntry_T      mac_entry;

        /* check if entry really expire
         */
        if (sys_time < hisam_entry.session_expire_time)
        {
            break;
        }

        mac_entry.mac_index = hisam_entry.mac_index;

        /* get MAC address entry
         */
        if (FALSE == NETACCESS_OM_GetSecureAddressEntryByIndex(&mac_entry))
        {
            continue;
        }

        /* get state machine
         */
        if ((SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(mac_entry.lport, &unit, &port, &trunk_id))
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
            && (SWCTRL_LPORT_TRUNK_PORT != SWCTRL_LogicalPortToUserPort(mac_entry.lport, &unit, &port, &trunk_id))
#endif
            )
        {
            continue;
        }

        if (FALSE == NETACCESS_OM_GetPortStateMachine(mac_entry.lport, &state_machine))
        {
            continue;
        }

        /* check running_port_mode
         */
        switch (state_machine.running_port_mode)
        {
            case NETACCESS_PORTMODE_NO_RESTRICTIONS:
            case NETACCESS_PORTMODE_CONTINUOS_LEARNING:
            case NETACCESS_PORTMODE_AUTO_LEARN:
            case NETACCESS_PORTMODE_SECURE:
            case NETACCESS_PORTMODE_PORT_SECURITY:

                if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
                    printf("\r\n[%s] update expire time to MAXIMUM", __FUNCTION__);

                mac_entry.session_expire_time = NETACCESS_MAX_SESSION_EXPIRE_TIME;

                /* update MAC address entry
                 */
                if (FALSE == NETACCESS_OM_UpdateSecureAddressEntryByIndex(&mac_entry))
                {
                    if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
                        printf("\r\n[%s] NETWORKACCESS_OM_UpdateSecureAddressEntryByIndex() failed", __FUNCTION__);
                    return FALSE;
                }
                continue;

            case NETACCESS_PORTMODE_USER_LOGIN:
            case NETACCESS_PORTMODE_USER_LOGIN_SECURE:
            case NETACCESS_PORTMODE_MAC_ADDRESS_WITH_RADIUS:
            case NETACCESS_PORTMODE_MAC_ADDRESS_OR_USER_LOGIN_SECURE:
            case NETACCESS_PORTMODE_MAC_ADDRESS_ELSE_USER_LOGIN_SECURE:
            case NETACCESS_PORTMODE_MAC_AUTHENTICATION:
            case NETACCESS_PORTMODE_DOT1X:
                /* check if need to do reauth or age out
                 */
                if (NETACCESS_ROWSTATUS_ACTIVE == mac_entry.authorized_status)
                {
                    /* authorized,check if MAC address is authenticating
                     */
                    if (1 == state_machine.port_security_sm.event_bitmap.is_authenticating)
                    {
                        continue;
                    }

                    if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
                    {
                        printf("\r\n[%s:%d] expiration of authoruzed mac: ", __FUNCTION__, __LINE__);
                        NETACCESS_VM_LocalPrintMacAddr(mac_entry.secure_mac);
                    }

                    /* check if preauthed
                     * update timer to MAX session expire time
                     */
                    if(1 == mac_entry.mac_flag.is_mac_filter_mac)
                    {
                        mac_entry.session_expire_time = NETACCESS_MAX_SESSION_EXPIRE_TIME;

                        /* update MAC address entry
                         */
                        if (FALSE == NETACCESS_OM_UpdateSecureAddressEntryByIndex(&mac_entry))
                        {
                            if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
                                printf("\r\n[%s] NETWORKACCESS_OM_UpdateSecureAddressEntryByIndex() failed", __FUNCTION__);
                            return FALSE;
                        }
                        continue;
                    }

                    /* mac-authentication mac address
                     * delete it when session timeout
                     */
                    if(1 == mac_entry.mac_flag.auth_by_rada)
                    {
                        /* delete mac entry
                         */
                        if (FALSE == NETACCESS_VM_DeleteAuthorizedUserByIndex(mac_entry.mac_index))
                        {
                        }
                        continue;
                    }

#if (SYS_CPNT_DOT1X == TRUE)
                    /* dot1x authorized mac address
                     * if reauth is enabled of dot1x, doesn't handle the mac's re-auth
                     * it will be done by dot1x re-auth sm
                     */
                    if (    (VAL_dot1xPaePortReauthenticate_true == DOT1X_OM_Get_PortReAuthEnabled(mac_entry.lport))
                         && (1 == mac_entry.mac_flag.authorized_by_dot1x)
                       )
                    {
                        continue;
                    }

                    /* dot1x authorized mac address
                     * if reauth is disabled of dot1x, update the session expire time to secure
                     * table.
                     */
                    if (    (VAL_dot1xPaePortReauthenticate_false == DOT1X_OM_Get_PortReAuthEnabled(mac_entry.lport))
                         && (1 == mac_entry.mac_flag.authorized_by_dot1x)
                       )
                    {
                        UI32_T sys_time;

                        SYS_TIME_GetRealTimeBySec(&sys_time);
                        mac_entry.session_expire_time = sys_time + mac_entry.session_time;

                        NETACCESS_OM_UpdateSecureAddressEntryByIndex(&mac_entry);
                        continue;
                    }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

                    /* session timeout, do re-authentication
                     */
                    state_machine.port_security_sm.event_bitmap.reauth = 1;
                    state_machine.port_security_sm.event_bitmap.eap_packet = mac_entry.mac_flag.eap_packet;
                }
                else
                {
                    /* unauthorized, check if need ageout
                     */
                    if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
                    {
                        printf("\r\n[%s%d] expiration of unauthoruzed mac: ", __FUNCTION__, __LINE__);
                        NETACCESS_VM_LocalPrintMacAddr(mac_entry.secure_mac);
                    }

                    /* get intrusion action
                     */
                    if (FALSE == NETACCESS_OM_GetSecureIntrusionAction(mac_entry.lport, &intrusion_action))
                    {
                        continue;
                    }

                    /* check with intrusion action
                     */
                    if (NETACCESS_INTRUSIONACTION_ALLOW_DEFAULT_ACCESS == intrusion_action)
                    {
                        /* check if MAC address authenticating
                         */
                        if (1 == state_machine.port_security_sm.event_bitmap.is_authenticating)
                        {
                            continue;
                        }

                        /* 3Com 700-49-007 MBNA, page 11
                         * 2.3.1.2  Allow Default Access Re-authentication
                         * On expiry of the session timer, the MAC address SHALL be re-authenticated in the
                         * same manner as an authorised host.
                         */
                        state_machine.port_security_sm.event_bitmap.reauth = 1;
                        state_machine.port_security_sm.event_bitmap.eap_packet = mac_entry.mac_flag.eap_packet;
                    }
                    else
                    {
                        /* 3Com 700-49-007 MBNA, page 11
                         * 2.3.1.3  Blocking Re-authentication
                         * On expiry of the hold-off timer, the MAC address SHALL be removed from the filter
                         * database and the next occurrence shall be re-authenticated from scratch.
                         */
                        if (FALSE == NETACCESS_VM_DeleteAuthorizedUserByIndex(mac_entry.mac_index))
                        {
                        }

                        continue;
                    }
                }
                break;

            case NETACCESS_PORTMODE_USER_LOGIN_WITH_OUI: /* not support this mode */
            default:
                if (FALSE == NETACCESS_VM_DeleteAuthorizedUserByIndex(mac_entry.mac_index))
                {
                }

                continue;
        }

        memcpy(state_machine.port_security_sm.authenticating_mac, mac_entry.secure_mac, SYS_ADPT_MAC_ADDR_LEN);

        /* run state machine
         */
        if (FALSE == NETACCESS_VM_RunSecurePortModeSM(mac_entry.lport, &state_machine)
           &&(NULL != state_machine.port_security_sm.cookie)
           )
        {
            NETACCESS_VM_AuthResult_T auth_result;

            auth_result.result = SYS_CALLBACK_MGR_AUTH_UNAUTHENTICATED;
            auth_result.vid = 0;

            NETACCESS_VM_LocalPassCookieToNextCSC(lport,
                                                  &state_machine,
                                                  &auth_result);
        }
    }

    /* process disablePortTemporarily timer */
    while (TRUE == NETACCESS_OM_GetFirstPortDisableTimer(&lport, &expire_time))
    {
        if (sys_time < expire_time)
            break;

        NETACCESS_OM_DestroyFirstPortDisableTimer();

        if (FALSE == SYS_CALLBACK_MGR_SetPortStatusCallback(SYS_MODULE_NETACCESS, lport, TRUE, SWCTRL_PORT_STATUS_SET_BY_NETACCESS_LINK_DETECTION))
        {
        }
    }

    if (NETACCESS_OM_DEBUG_VM_TMR & debug_flag)
        printf("\r\n[%s] done", __FUNCTION__);

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessRadiusMsg
 * ---------------------------------------------------------------------
 * PURPOSE: process radius msg
 * INPUT:  radius_msg.
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessRadiusMsg(NETACCESS_RADIUS_MSGQ_T *radius_msg)
{
    NETACCESS_OM_StateMachine_T state_machine;
    UI32_T                      debug_flag = 0, lport;
    BOOL_T                      dynamic_vlan_status;
    BOOL_T                      dynamic_qos_status;
    BOOL_T                      authorized_result = TRUE;
    BOOL_T                      ret;

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* check if input is valid and get port state machine
     */
    if ((NULL == radius_msg) ||
        (NULL == radius_msg->m_radius_data) ||
        (FALSE == NETACCESS_OM_GetPortStateMachine(radius_msg->m_radius_data->lport, &state_machine)))
    {
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] null pointer or bad lport", __FUNCTION__);

        return FALSE;
    }

    lport = radius_msg->m_radius_data->lport;

    NETACCESS_VM_DEBUG_PRINT_FUNCTION_START();

    /* setup event flag
     */
    if (TRUE == radius_msg->m_radius_data->authorized_result)
    {
        NETACCESS_OM_GetDynamicVlanStatus(lport, &dynamic_vlan_status);
        if (TRUE == dynamic_vlan_status)
        {
            authorized_result &= NETACCESS_VM_LocalIsValidVlanList(lport,
                                                                   radius_msg->m_radius_data->authorized_vlan_list,
                                                                   &state_machine);

        }

        NETACCESS_OM_GetDynamicQosStatus(lport, &dynamic_qos_status);
        if ((TRUE == dynamic_qos_status) && (TRUE == authorized_result))
        {
            authorized_result &= NETACCESS_VM_LocalIsValidQosProfiles(radius_msg->m_radius_data->lport,
                                                                      radius_msg->m_radius_data->authorized_qos_list,
                                                                      &state_machine);
        }

        if (TRUE == authorized_result)
        {
            state_machine.port_security_sm.event_bitmap.rada_success = 1;
        }
        else
        {
            state_machine.port_security_sm.event_bitmap.rada_fail = 1;
        }
    }
    else
    {
        state_machine.port_security_sm.event_bitmap.rada_fail = 1;
    }

    /* duplicate pointer to om
     */
    state_machine.port_security_sm.radius_msg = radius_msg;

    /* run state machine
     */
    ret = NETACCESS_VM_RunSecurePortModeSM(lport, &state_machine);
    if (FALSE == ret)
    {
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] NETACCESS_VM_RunSecurePortModeSM(%lu) failed", __FUNCTION__, lport);

        if (NULL != state_machine.port_security_sm.cookie)
        {
            NETACCESS_VM_AuthResult_T auth_result;

            auth_result.result = SYS_CALLBACK_MGR_AUTH_UNAUTHENTICATED;
            auth_result.vid = 0;

            NETACCESS_VM_LocalPassCookieToNextCSC(lport,
                                                  &state_machine,
                                                  &auth_result);
        }

        /* when state machine fail, should stop any authentication
         */
        if (FALSE == NETACCESS_OM_StopStateMachineDoAuthentication(lport)) /* to avoid state machine to be checkmated */
        {
        }
    }

    /* must clean up new_mac_msg no matter ret is TURE or FALSE
     */
    if (FALSE == NETACCESS_OM_ClearStateMachineRadiusMsg(lport))
    {
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] NETACCESS_OM_ClearStateMachineRadiusMsg(%lu) failed", __FUNCTION__, lport);
        ret = FALSE;
    }

    return ret;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessDot1xMsg
 * ---------------------------------------------------------------------
 * PURPOSE: process dot1x msg
 * INPUT:  dot1x_msg.
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessDot1xMsg(NETACCESS_DOT1X_MSGQ_T *dot1x_msg)
{
    BOOL_T      /*secure_port_security_control, */ret;
    UI32_T      debug_flag = 0, lport;

    NETACCESS_OM_StateMachine_T     state_machine;

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* check if input is valid and get state machine
     */
    if ((NULL == dot1x_msg) ||
        (NULL == dot1x_msg->m_dot1x_data) ||
        (FALSE == NETACCESS_OM_GetPortStateMachine(dot1x_msg->m_dot1x_data->lport, &state_machine)))
    {
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] null pointer or bad lport", __FUNCTION__);

        return FALSE;
    }

    lport = dot1x_msg->m_dot1x_data->lport;

    NETACCESS_VM_DEBUG_PRINT_FUNCTION_START();

    /* setup event flag */
    switch (dot1x_msg->m_dot1x_data->authorized_result)
    {
        case DOT1X_TYPE_AUTH_RESULT_SUCCESS:
            state_machine.port_security_sm.event_bitmap.dot1x_success = 1;
            break;
        case DOT1X_TYPE_AUTH_RESULT_FAIL:
            state_machine.port_security_sm.event_bitmap.dot1x_fail = 1;
            break;
        case DOT1X_TYPE_AUTH_RESULT_LOGOFF:
            state_machine.port_security_sm.event_bitmap.dot1x_logoff = 1;
            break;
        case DOT1X_TYPE_AUTH_RESULT_NO_EAPOL:
            state_machine.port_security_sm.event_bitmap.dot1x_no_eapol = 1;
            break;
        default:
            if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
                printf("[NETACCESS_VM_ProcessDot1xMsg] unknown authorized result(%d)\r\n", dot1x_msg->m_dot1x_data->authorized_result);

            state_machine.port_security_sm.event_bitmap.dot1x_fail = 1; /* treat it as fail */
            break;
    }

    /* duplicate pointer to om
     */
    state_machine.port_security_sm.dot1x_msg = dot1x_msg;

    /* EPR:ES4649-32-01139
     * copy authenticated MAC to state machine from dot1x
     */
    memcpy(state_machine.port_security_sm.authenticating_mac, dot1x_msg->m_dot1x_data->authorized_mac, SYS_ADPT_MAC_ADDR_LEN);

    /* run state machine
     */
    ret = NETACCESS_VM_RunSecurePortModeSM(lport, &state_machine);
    if (FALSE == ret)
    {
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] NETACCESS_VM_RunSecurePortModeSM(%lu) failed", __FUNCTION__, lport);

        if (NULL != state_machine.port_security_sm.cookie)
        {
            NETACCESS_VM_AuthResult_T auth_result;

            auth_result.result = SYS_CALLBACK_MGR_AUTH_UNAUTHENTICATED;
            auth_result.vid = 0;

            NETACCESS_VM_LocalPassCookieToNextCSC(lport,
                                                  &state_machine,
                                                  &auth_result);
        }

        /* when run state machine fail,should stop any authentication
         */
        if (FALSE == NETACCESS_OM_StopStateMachineDoAuthentication(lport))
        {
        }
    }

    /* must clean up new_mac_msg no matter ret is TURE or FALSE
     */
    if (FALSE == NETACCESS_OM_ClearStateMachineDot1xMsg(lport))
    {
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] NETACCESS_OM_ClearStateMachineDot1xMsg(%lu) failed", __FUNCTION__, lport);
        ret = FALSE;
    }

    return ret;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_DetectPortMove
 * ---------------------------------------------------------------------
 * PURPOSE: detect event (port move)
 * INPUT:  lport, new_mac
 * OUTPUT: None.
 * RETURN: TRUE -- should learn the new mac / FALSE -- should not learn this new mac
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_DetectPortMove(UI32_T lport, UI8_T *new_mac, UI32_T vid)
{
    UI32_T      debug_flag = 0;
    UI32_T      new_port_mode, original_lport, original_port_mode;
    UI32_T      unit, port, trunk_id;
    NETACCESS_OM_SecureKey_T        key;
    NETACCESS_OM_HISAMentry_T       hisam_entry;
    UI32_T tmp_vid;

    //NETACCESS_VM_CHECK_PSEC_CONTROL_MODE(TRUE); /* don't treat it as an error */

    /* get running port mode
     */
    if (FALSE == NETACCESS_OM_GetPortStateMachineRunningPortMode(lport, &new_port_mode))
    {
        return FALSE;
    }

    debug_flag = NETACCESS_OM_GetDebugFlag();

    tmp_vid = 0;

    /* check if same vid
     */
    while (TRUE == NETACCESS_VM_LocalGetNextMacFromAmtr(new_mac, &original_lport, &tmp_vid))
    {
        /* check if same vid and different port
         */
        if((tmp_vid != vid) || (original_lport == lport))
        {
            break;
        }

        /* get original port's mode
         */

        if ((SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(original_lport, &unit, &port, &trunk_id))
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
             && (SWCTRL_LPORT_TRUNK_PORT != SWCTRL_LogicalPortToUserPort(original_lport, &unit, &port, &trunk_id))
#endif
            )
        {
            continue;
        }

        if (FALSE == NETACCESS_OM_GetPortStateMachineRunningPortMode(original_lport, &original_port_mode))
        {
            continue;
        }

        /*
         */
        key.lport = original_lport;
        memcpy(key.secure_mac, new_mac, SYS_ADPT_MAC_ADDR_LEN);

        if(FALSE == NETACCESS_OM_GetHisamRecordBySecureKey(&key, &hisam_entry))
        {
            break;
        }

        /* check admin_configured_mac,
         * administrative configured mac has HIGHEST priority
         */
        if (TRUE == NETACCESS_OM_IsSecureAddressAdminConfigured(hisam_entry.mac_index))
        {
            if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
                printf("\r\n[%s] can't replace administrative configured mac", __FUNCTION__);
            return FALSE;
        }

        /* check port move issue
         */
        if (FALSE == NETACCESS_VM_LocalCheckAutoPortMoveIssue(original_port_mode, new_port_mode, FALSE, FALSE))
        {
            if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
                printf("\r\n[%s] the address is already in the table for a port(%lu)", __FUNCTION__, key.lport);
            return FALSE;
        }

        /* port move happen,delete old MAC address
         */
        if (FALSE == NETACCESS_VM_DeleteAuthorizedUserByIndex(hisam_entry.mac_index))
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_DeleteMacFromAmtrByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : Delete the specified mac address from AMTR by lport
 * INPUT    : lport, mac
 * OUTPUT   : None
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTE     : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_VM_DeleteMacFromAmtrByPort(UI32_T lport, UI8_T *mac)
{
    AMTR_TYPE_AddrEntry_T   addr_entry;
    BOOL_T                  ret = TRUE;

    /* check if input is valid
     */
    if (NULL == mac)
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("Null pointer");
        return FALSE;
    }

    /* For each VLAN, delete the specified MAC address by port.
     */
    memset(&addr_entry, 0, sizeof(addr_entry));
    addr_entry.ifindex = lport;
    memcpy(addr_entry.mac, mac, SYS_ADPT_MAC_ADDR_LEN);
    while (TRUE == AMTR_MGR_GetNextMVAddrEntry(&addr_entry, AMTR_MGR_GET_ALL_ADDRESS))
    {
        if ((addr_entry.ifindex != lport) || (memcmp(mac, addr_entry.mac, SYS_ADPT_MAC_ADDR_LEN) != 0))
        {
            break;
        }

        if (addr_entry.source != NETACCESS_VM_SecureAddrSourceType())
        {
            continue;
        }

        ret &= AMTR_MGR_DeleteAddr(addr_entry.vid, addr_entry.mac);
    }

    if (ret == FALSE)
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("Delete some MAC failed");
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_VM_DeleteAllDynamicMacFromAmtrByPort
 *-------------------------------------------------------------------------
 * PURPOSE : Delete all dynamic MAC address from MAC address table by port.
 * INPUT   : lport -- logic port number.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : All actions should be completed after this function returned.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_DeleteAllDynamicMacFromAmtrByPort(UI32_T lport)
{
    UI32_T                  orig_psec_owner, orig_psec_status;
    AMTR_TYPE_AddrEntry_T   addr_entry;
    BOOL_T                  ret = TRUE;

    if(TRUE == SWCTRL_IsSecurityPort(lport, &orig_psec_owner))
    {
        orig_psec_status = VAL_portSecPortStatus_enabled;
    }
    else
    {
        orig_psec_status = VAL_portSecPortStatus_disabled;
        orig_psec_owner  = SWCTRL_PORT_SECURITY_ENABLED_BY_NONE;
    }

    /* Disable auto learning
     */
    if (FALSE == SWCTRL_SetPortSecurityStatus(lport, VAL_portSecPortStatus_enabled, SWCTRL_PORT_SECURITY_ENABLED_BY_NETACCESS))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("SWCTRL_SetPortSecurityStatus(enable) failed");
        return FALSE;
    }

    /* Delete all dynamic MAC by lport
     */
    memset(&addr_entry, 0, sizeof(addr_entry));
    addr_entry.ifindex = lport;
    while (TRUE == AMTR_MGR_GetNextIfIndexAddrEntry(&addr_entry, AMTR_MGR_GET_ALL_ADDRESS))
    {
        if (addr_entry.ifindex != lport)
        {
            break;
        }

        if (addr_entry.source == AMTR_TYPE_ADDRESS_SOURCE_LEARN ||
            addr_entry.source == AMTR_TYPE_ADDRESS_SOURCE_SECURITY)
        {
            NETACCESS_VM_DEBUG_PRINT_TRACE("Delete MAC(%02X%02X%02X-%02X%02X%02X,vid=%d)",
                addr_entry.mac[0], addr_entry.mac[1], addr_entry.mac[2],
                addr_entry.mac[3], addr_entry.mac[4], addr_entry.mac[5],
                addr_entry.vid);
            ret &= AMTR_MGR_DeleteAddr(addr_entry.vid, addr_entry.mac);
        }
    }

    /* Restore port security status
     */
    if(FALSE == SWCTRL_SetPortSecurityStatus(lport, orig_psec_status, orig_psec_owner))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("SWCTRL_SetPortSecurityStatus() failed");
        return FALSE;
    }

    if (ret == FALSE)
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("Delete some MAC failed");
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_DeleteAuthorizedUserByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Destroy a mac address by mac_index (amtr & om & hisam)
 * INPUT    : mac_index
 * OUTPUT   : None
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTE     : This function will destory a MAC address from AMTR and OM.
 *
 *            The port that the deleting MAC is on should be restored to
 *            static VLAN and QoS configuration, if there have no other
 *            dynamic authorized MAC on this port.
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_VM_DeleteAuthorizedUserByIndex(UI32_T mac_index)
{
    NETACCESS_OM_SecureMacEntry_T   mac_entry;

    /* get MAC address entry,port mode and intrusion action
     */
    mac_entry.mac_index = mac_index;
    if (FALSE == NETACCESS_OM_GetSecureAddressEntryByIndex(&mac_entry))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("Wrong mac_index(%lu)", mac_index);
        return FALSE;
    }

    if (FALSE == NETACCESS_VM_LocalDeleteSecureMacAddr(&mac_entry))
    {
        return FALSE;
    }

    /* check if no more learnt address,restore VLAN and QoS
     */
    if (0 == NETACCESS_VM_GetSecureNumberAddressesStoredWithoutMacFilter(mac_entry.lport))
    {
        if (FALSE == NETACCESS_VM_ResetVlanQos2Default(mac_entry.lport))
        {
            return FALSE;
        }
    }

    return TRUE;
}

#if 0
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_DeletePortMoveMac
 *-------------------------------------------------------------------------
 * PURPOSE  : destroy a port move mac address (amtr & om & hisam)
 * INPUT    : new_lport, ori_mac_index
 * OUTPUT   : None
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTE     : if mac is not existed, return FALSE
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_VM_DeletePortMoveMac(UI32_T new_lport, UI32_T ori_mac_index)
{
    UI32_T unit, port, trunk_id, port_mode, intrusion_action;
    UI32_T debug_flag = 0;
    NETACCESS_OM_SecureMacEntry_T      mac_entry;

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* get MAC address entry,port mode and intrusion action
     */
    mac_entry.mac_index = ori_mac_index;
    if ((FALSE == NETACCESS_OM_GetSecureAddressEntryByIndex(&mac_entry)) ||
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
        (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(mac_entry.lport, &unit, &port, &trunk_id) &&
         SWCTRL_LPORT_TRUNK_PORT != SWCTRL_LogicalPortToUserPort(mac_entry.lport, &unit, &port, &trunk_id)) ||
#else
        (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(mac_entry.lport, &unit, &port, &trunk_id)) ||
#endif
        (FALSE == NETACCESS_OM_GetPortStateMachineRunningPortMode(mac_entry.lport, &port_mode)) ||
        (FALSE == NETACCESS_OM_GetSecureIntrusionAction(mac_entry.lport, &intrusion_action)))
    {
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] check input failed", __FUNCTION__);
        return FALSE;
    }

    /* update om & hisam
     */
    if (FALSE == NETACCESS_OM_DeleteSecureAddressEntryByIndex(ori_mac_index))
    {
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] NETACCESS_OM_DeleteSecureAddressEntryByIndex failed", __FUNCTION__);
        return FALSE;
    }

    /* if mac had been add to AMTR, replace from AMTR
     */
    if (1 == mac_entry.mac_flag.write_to_amtr)
    {
        /* delete port move MAC address from AMTR
         */
        if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
            printf("\r\n[%s] delete port move mac to new port:%ld", __FUNCTION__, new_lport);

        if (FALSE == NETACCESS_VM_DeletePortMoveMacFromAmtrByPort(mac_entry.lport, new_lport, mac_entry.secure_mac))
        {
            if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
                printf("\r\n[%s] NETACCESS_VM_DeleteMacFromAmtrByPort failed", __FUNCTION__);
            return FALSE;
        }
    }

    /* check port mode
     */
    switch (port_mode)
    {
        case NETACCESS_PORTMODE_NO_RESTRICTIONS:
        case NETACCESS_PORTMODE_CONTINUOS_LEARNING:
        case NETACCESS_PORTMODE_AUTO_LEARN:
        case NETACCESS_PORTMODE_SECURE:
        case NETACCESS_PORTMODE_MAC_AUTHENTICATION:
        case NETACCESS_PORTMODE_PORT_SECURITY:
            break;
        case NETACCESS_PORTMODE_MAC_ADDRESS_WITH_RADIUS:
        case NETACCESS_PORTMODE_MAC_ADDRESS_OR_USER_LOGIN_SECURE:
        case NETACCESS_PORTMODE_MAC_ADDRESS_ELSE_USER_LOGIN_SECURE:
            /* check 1x authorized mac
             */
            if (0 == mac_entry.mac_flag.authorized_by_dot1x)
            {
                break;
            }

            /* clear flag
             */
            if (FALSE == NETACCESS_OM_ClearStateMachineDot1xLogonFlag(mac_entry.lport))
            {
                return FALSE;
            }

            break;

        case NETACCESS_PORTMODE_USER_LOGIN_WITH_OUI: /* not support this mode */
        default:
            NETACCESS_VM_DEBUG_PRINT_ERROR("Netaccess: bad port mode(%lu) \r\n", port_mode);
            return FALSE;
    }

    /* Authorized MAC or allowDefaultAccess unauthorized MAC
     * need to do reCalc VLAN & Qos
     */
    if ((NETACCESS_ROWSTATUS_ACTIVE == mac_entry.authorized_status) ||
        (NETACCESS_INTRUSIONACTION_ALLOW_DEFAULT_ACCESS == intrusion_action))
    {
        /* re-calc VLAN & QoS */
        if (FALSE == NETACCESS_VM_RecalculateVlanQos(mac_entry.lport, NULL))
        {
            if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
                printf("\r\n[%s] NETACCESS_VM_RecalculateVlanQos failed", __FUNCTION__);

            return FALSE;
        }
    }

    return TRUE;
}
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_DeleteAllAuthorizedUserByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : Delete all authorized user from AMTR & OM by lport
 * INPUT    : lport
 * OUTPUT   : None
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_DeleteAllAuthorizedUserByPort(UI32_T lport)
{
    NETACCESS_OM_SecureMacEntry_T   entry;
    BOOL_T                          ret = TRUE;

    /* get secure mac entry by lport
     */
    memset(&entry, 0, sizeof(NETACCESS_OM_SecureMacEntry_T));
    entry.lport = lport;
    while(TRUE == NETACCESS_OM_GetNextSecureAddressEntry(&entry))
    {
        /* if not same lport,means no record for this port,just return TRUE
         */
        if(lport != entry.lport)
        {
            break;
        }

        ret &= NETACCESS_VM_LocalDeleteSecureMacAddr(&entry);
    }

    ret &= NETACCESS_VM_ResetVlanQos2Default(lport);

    return ret;
}

BOOL_T NETACCESS_VM_ApplyVlanQos(
    UI32_T lport,
    NETACCESS_OM_StateMachine_T *state_machine
    )
{
    char *dynamic_vlan = NETACCESS_VM_RETURNED_VLAN(state_machine);
    char *dynamic_qos  = NETACCESS_VM_RETURNED_QOS(state_machine);

    /* The dot1x users who are no need to authenticate by radius server are unnecessary to apply
     * dynamic vlan or qos
     */
    if (   (NULL == dynamic_vlan)
        || (NULL == dynamic_qos))
    {
        return TRUE;
    }

    if (FALSE == NETACCESS_VM_LocalSetToAutoVlan(lport, dynamic_vlan))
    {
        return FALSE;
    }

    if (FALSE == NETACCESS_VM_LocalApplyQosProfiles(lport, dynamic_qos, state_machine))
    {
        return FALSE;
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_ResetVlanQos2Default
 *-------------------------------------------------------------------------
 * PURPOSE  : reset a port VLAN and QoS to administrative configuration
 * INPUT    : lport
 * OUTPUT   : None
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTE     : none
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ResetVlanQos2Default(UI32_T lport)
{
    BOOL_T ret = TRUE;

    ret = NETACCESS_VM_LeaveGuestVlan(lport);

    ret &= NETACCESS_VM_LocalLeaveAuthorizedVlan(lport);

    ret &= NETACCESS_VM_LocalResetQos2Default(lport);

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_SetAllMacExpiredByLport
 *-------------------------------------------------------------------------
 * PURPOSE  : let all mac on lport expire so that they will be reauthenticated
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE -- succeeded / FALSE -- failed
 * NOTE     : None
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_VM_SetAllMacExpiredByLport(UI32_T lport)
{
    UI32_T      debug_flag = 0;
    UI32_T      unit, port, trunk_id, sys_time;

    NETACCESS_OM_SecureMacEntry_T      mac_entry;

    debug_flag = NETACCESS_OM_GetDebugFlag();

#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) &&
        SWCTRL_LPORT_TRUNK_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
#else
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
#endif
        return FALSE;

    NETACCESS_VM_DEBUG_PRINT_FUNCTION_START();

    SYS_TIME_GetRealTimeBySec(&sys_time);

    /* set every MAC on lport expired */
    memset(&mac_entry, 0, sizeof(mac_entry));
    mac_entry.lport = lport;
    while (TRUE == NETACCESS_OM_GetNextSecureAddressEntry(&mac_entry))
    {
        if (lport != mac_entry.lport)
        {
            break;
        }

        mac_entry.session_expire_time = sys_time;

        if (FALSE == NETACCESS_OM_UpdateSecureAddressEntryByIndex(&mac_entry))
            return FALSE;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_IsValidVlanList
 * ---------------------------------------------------------------------
 * PURPOSE: This function validates the VLAN list.
 * INPUT  : lport       -- logic port number
 *          vlan_list_p -- VLAN string list
 * OUTPUT : None.
 * RETURN : TRUE  -- valid
 *          FALSE -- invalid
 * NOTES :  This function checks the port mode and verifies the VLAN list.
 *          After the check. If the VLAN list can be applied to port,
 *          the return_vlan_change flag should be set.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_IsValidVlanList(UI32_T lport, const char *vlan_list_p)
{
    NETACCESS_OM_StateMachine_T state_machine;
    BOOL_T                      dynamic_vlan_status;

    if (FALSE == NETACCESS_OM_GetDynamicVlanStatus(lport, &dynamic_vlan_status))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("Wrong lport(%lu)", lport);
        return FALSE;
    }

    /* If dynamic VLAN assignment is disabled
     */
    if (FALSE == dynamic_vlan_status)
    {
        return TRUE;
    }

    if (FALSE == NETACCESS_OM_GetPortStateMachine(lport, &state_machine))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("NETACCESS_OM_GetPortStateMachine failed");
        return FALSE;
    }

    if (FALSE == NETACCESS_VM_LocalIsValidVlanList(lport, vlan_list_p, &state_machine))
    {
        return FALSE;
    }

    /* Update state machine, if check okay.
     * If the VLAN list can be applied to port, the return_vlan_change flag will be set.
     */
    if (FALSE == NETACCESS_OM_SetPortStateMachine(lport, &state_machine))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("NETACCESS_OM_SetPortStateMachine failed");
        return FALSE;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_IsValidQosProfiles
 * ---------------------------------------------------------------------
 * PURPOSE: This function validates the QoS profiles.
 * INPUT  : lport           -- logic port number
 *          str_profiles    -- QoS profiles string
 * OUTPUT : None.
 * RETURN : TRUE  -- valid
 *          FALSE -- invalid
 * NOTES :  This function checks the port mode and verifies the QoS profiles.
 *          After the check. If the 802.1p profile can be applied to port,
 *          the return_default_port_priority_changed flag should be set.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_IsValidQosProfiles(UI32_T lport, const char *str_profiles)
{
#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
    NETACCESS_OM_StateMachine_T state_machine;
    BOOL_T                      dynamic_qos_status;

    if (FALSE == NETACCESS_OM_GetDynamicQosStatus(lport, &dynamic_qos_status))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("Wrong lport(%lu)", lport);
        return FALSE;
    }

    /* If dynamic VLAN assignment is disabled
     */
    if (FALSE == dynamic_qos_status)
    {
        return TRUE;
    }

    if (FALSE == NETACCESS_OM_GetPortStateMachine(lport, &state_machine))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("NETACCESS_OM_GetPortStateMachine failed");
        return FALSE;
    }

    if (FALSE == NETACCESS_VM_LocalIsValidQosProfiles(lport, str_profiles, &state_machine))
    {
        return FALSE;
    }

    /* Update state machine, if check okay.
     * If the 802.1p profile can be applied to port,
     * the return_default_port_priority_changed flag should be set.
     */
    if (FALSE == NETACCESS_OM_SetPortStateMachine(lport, &state_machine))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("NETACCESS_OM_SetPortStateMachine failed");
        return FALSE;
    }
#endif /* #if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE) */
    return TRUE;
}

#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_VM_SetMacAddressAgingMode
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
BOOL_T NETACCESS_VM_SetMacAddressAgingMode(UI32_T mode)
{
    UI32_T  orig_mode;
    UI32_T  lport, src_unit, src_port, trunk_id;
    UI32_T  psec_mac_count;
    UI32_T  psec_status;
    UI32_T  run_mode;

    if ((mode != VAL_networkAccessAging_enabled)
        &&  (mode != VAL_networkAccessAging_disabled)
       )
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("invalid aging mode(%ld)", mode);
        return FALSE;
    }

    if (FALSE == NETACCESS_OM_GetMacAddressAgingMode(&orig_mode))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("NETACCESS_OM_GetMacAddressAgingStatus() failed");
        return FALSE;
    }

    if (mode == orig_mode)
    {
        return TRUE;
    }

    /* For all secure port, re-learn all secure MAC address
     * and reset the life time of MAC address to port.
     */
    for (lport = 1; lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT; lport++)
    {
        if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &src_unit, &src_port, &trunk_id)
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
            && SWCTRL_LPORT_TRUNK_PORT != SWCTRL_LogicalPortToUserPort(lport, &src_unit, &src_port, &trunk_id)
#endif
           )
        {
            continue;
        }

        if (FALSE == PSEC_OM_GetMaxMacCount(lport, &psec_mac_count)
            || FALSE == PSEC_OM_GetPortSecurityStatus(lport, &psec_status)
            || FALSE == NETACCESS_OM_GetPortStateMachineRunningPortMode(lport, &run_mode)
            )
        {
            continue;
        }

        if (0 < psec_mac_count
            || psec_status == VAL_portSecPortStatus_enabled
            || run_mode != NETACCESS_PORTMODE_NO_RESTRICTIONS
            )
        {
            AMTR_MGR_PortInfo_T port_info;

            AMTR_MGR_DeleteAddrBySourceAndLPort(lport, NETACCESS_VM_SecureAddrSourceType() );

            if (TRUE == AMTR_OM_GetPortInfo(lport, &port_info))
            {
                port_info.life_time = NETACCESS_VM_SecureAddrLifeTime(
                    mode == VAL_networkAccessAging_enabled ? TRUE : FALSE
                    );

                if (FALSE == AMTR_MGR_SetPortInfo(lport, &port_info))
                {
                    NETACCESS_VM_DEBUG_PRINT_ERROR("AMTR_OM_GetPortInfo(%lu) failed", lport);
                }
            }
            else
            {
                NETACCESS_VM_DEBUG_PRINT_ERROR("AMTR_OM_GetPortInfo(%lu) failed", lport);
            }
        }

    }

    return NETACCESS_OM_SetMacAddressAgingMode(mode);
}
#endif /* #if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE) */

/* LOCAL SUBPROGRAM BODIES
 */
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalInitializeVM
 * ---------------------------------------------------------------------
 * PURPOSE: initialize VM
 * INPUT:  none.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalInitializeVM()
{
    UI32_T lport;
    UI32_T debug_flag = 0;
    NETACCESS_PortMode_T            port_mode;
    NETACCESS_OM_StateMachine_T     state_machine;

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* initialize all state machine by om port mode */
    for (lport = 1; SYS_ADPT_TOTAL_NBR_OF_LPORT >= lport; ++lport)
    {
        if (FALSE == SWCTRL_LogicalPortExisting(lport)) /* no need to initialize it */
            continue;

        if ((FALSE == NETACCESS_OM_GetSecurePortMode(lport, &port_mode)) ||
            (FALSE == NETACCESS_OM_GetPortStateMachine(lport, &state_machine)))
        {
            if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
                printf("\r\n[%s] get (%lu) port mode or state machine failed", __FUNCTION__, lport);
            continue;
        }

        state_machine.new_port_mode = port_mode;

        NETACCESS_VM_RUN_PORT_MODE_CHANGE_SM(lport, &state_machine);
    }

    if (NETACCESS_OM_DEBUG_VM_IFO & debug_flag)
        printf("\r\n[%s] done", __FUNCTION__);

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_EnterSecurePortMode
 * ---------------------------------------------------------------------
 * PURPOSE: start the state machine (initialize)
 * INPUT:  state_machine
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_EnterSecurePortMode(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    UI32_T      debug_flag = 0;

    NETACCESS_VM_DEBUG_PRINT_FUNCTION_START();

    /* check if input is valid
     */
    if (NULL == state_machine)
    {
        return FALSE;
    }

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* call function pointer
     */
    if(NULL != port_mode_fun_p[state_machine->new_port_mode-1].enter_port_mode)
    {
        NETACCESS_VM_ResetVlanQos2Default(lport);
        return port_mode_fun_p[state_machine->new_port_mode-1].enter_port_mode(lport, state_machine);
    }

    return TRUE;
}

static BOOL_T NETACCESS_VM_LocalExitSecurePortMode2NewMode(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    UI32_T      debug_flag = 0;

    NETACCESS_VM_DEBUG_PRINT_FUNCTION_START();

    /* check new_port_mode
     */
    switch (state_machine->new_port_mode)
    {
        case NETACCESS_PORTMODE_NO_RESTRICTIONS:
        case NETACCESS_PORTMODE_CONTINUOS_LEARNING:
        case NETACCESS_PORTMODE_SECURE:
            /* 3FC458, page 10
             * The addresses in the secureAddressTable are preserved unchanged when the
             * securePortMode of the port is changed to NoRestrictions, ContinuousLearning or Secure.
             */
            break;

        case NETACCESS_PORTMODE_AUTO_LEARN:
            /* 3FC458, page 10
             * The agent will delete all the addresses for the port when the port is switched into
             * the 'Auto Learn' or any of the 'User Login' or RADA modes.
             */
            if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
                printf("\r\n[%s] delete all the addresses for the port(%lu)", __FUNCTION__, lport);

            if (FALSE == NETACCESS_VM_DeleteAllAuthorizedUserByPort(lport))
            {
            }

            /* delete from om & hisam
             */
            if (FALSE == NETACCESS_OM_DeleteAllLearnedSecureAddressByPort(lport))
            {
            }
            break;

        case NETACCESS_PORTMODE_USER_LOGIN:
        case NETACCESS_PORTMODE_USER_LOGIN_SECURE:
        case NETACCESS_PORTMODE_MAC_ADDRESS_WITH_RADIUS:
        case NETACCESS_PORTMODE_MAC_ADDRESS_OR_USER_LOGIN_SECURE:
        case NETACCESS_PORTMODE_MAC_ADDRESS_ELSE_USER_LOGIN_SECURE:
        case NETACCESS_PORTMODE_MAC_AUTHENTICATION:
        case NETACCESS_PORTMODE_PORT_SECURITY:
        case NETACCESS_PORTMODE_DOT1X:
            /* 3FC458, page 10
             * The agent will delete all the addresses for the port when the port is switched into
             * the 'Auto Learn' or any of the 'User Login' or RADA modes.
             */
            if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
                printf("\r\n[%s] delete all the addresses for the port(%lu)", __FUNCTION__, lport);

            /* delete all mac added to AMTR
             */
            if (FALSE == NETACCESS_VM_DeleteAllAuthorizedUserByPort(lport))            {
            }

            break;

        case NETACCESS_PORTMODE_USER_LOGIN_WITH_OUI: /* not support this mode */
        default:
            NETACCESS_VM_DEBUG_PRINT_ERROR("Wrong port mode(%lu)", state_machine->running_port_mode);
            return FALSE;
    }
    return TRUE;
}

static BOOL_T NETACCESS_VM_LocalExitSecurePortModeFromRunningMode(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    NETACCESS_VM_DEBUG_PRINT_FUNCTION_START();

    /* check running_port_mode
     */
    switch (state_machine->running_port_mode)
    {
        case NETACCESS_PORTMODE_USER_LOGIN:
        case NETACCESS_PORTMODE_NO_RESTRICTIONS:
            if (FALSE == NETACCESS_VM_DeleteAllDynamicMacFromAmtrByPort(lport))
            {
            }
            break;

        case NETACCESS_PORTMODE_MAC_AUTHENTICATION:
            if (FALSE == NETACCESS_VM_DeleteAllAuthorizedUserByPort(lport))
            {
            }
            break;

        case NETACCESS_PORTMODE_MAC_ADDRESS_OR_USER_LOGIN_SECURE:
        case NETACCESS_PORTMODE_DOT1X:
            NETACCESS_VM_LocalExitDot1xMode(lport, state_machine);

            /* fall down
             */
        case NETACCESS_PORTMODE_PORT_SECURITY:
            if (FALSE == NETACCESS_VM_DeleteAllAuthorizedUserByPort(lport))
            {
            }
            break;

        case NETACCESS_PORTMODE_CONTINUOS_LEARNING:
        case NETACCESS_PORTMODE_AUTO_LEARN:
        case NETACCESS_PORTMODE_SECURE:
        case NETACCESS_PORTMODE_USER_LOGIN_SECURE:
        case NETACCESS_PORTMODE_MAC_ADDRESS_WITH_RADIUS:
        case NETACCESS_PORTMODE_MAC_ADDRESS_ELSE_USER_LOGIN_SECURE:
            break;

        case NETACCESS_PORTMODE_USER_LOGIN_WITH_OUI: /* not support this mode */
        default:
            NETACCESS_VM_DEBUG_PRINT_ERROR("Wrong port mode(%lu)", state_machine->running_port_mode);
            return FALSE;
    }
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ExitSecurePortMode
 * ---------------------------------------------------------------------
 * PURPOSE: exit the state machine (cleanup all effects)
 * INPUT:  none.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_ExitSecurePortMode(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    UI32_T      debug_flag = 0, unit, port, trunk_id;

    NETACCESS_VM_DEBUG_PRINT_FUNCTION_START();

    if ((NULL == state_machine) ||
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
        (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) &&
         SWCTRL_LPORT_TRUNK_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id)))
#else
        (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id)))
#endif
    {
        return FALSE;
    }

    debug_flag = NETACCESS_OM_GetDebugFlag();

    NETACCESS_VM_LocalExitSecurePortMode2NewMode(lport, state_machine);

    /* no matter what port mode run, remove applied VLAN and QoS */
    if (FALSE == NETACCESS_VM_ResetVlanQos2Default(lport))
        return FALSE;

/*
ERP:ES4827G-FLF-ZZ-00290
Problem:   Port Security: Enable port securiy will clear the static mac.
rootcasue: system will clear all mac when change secure mode in 'netaccess'
sloution:  don't clear static mac when change secure mode to 'port security'
File:      netaccess_vm.c
*/
    if (state_machine->new_port_mode!=NETACCESS_PORTMODE_PORT_SECURITY)
    {
        NETACCESS_VM_LocalExitSecurePortModeFromRunningMode(lport, state_machine);
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_RunSecurePortModeSM
 * ---------------------------------------------------------------------
 * PURPOSE: run state machine (tran & exec)
 * INPUT:  none.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_RunSecurePortModeSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    NETACCESS_VM_DEBUG_PRINT_FUNCTION_START();

    /* check if input is valid
     */
    if (NULL == state_machine)
    {
        return FALSE;
    }

    /* run state machine
     */
    switch (state_machine->running_port_mode)
    {
        case NETACCESS_PORTMODE_NO_RESTRICTIONS:
            NETACCESS_VM_RUN_NO_RESTRICTION_SM(lport, state_machine);
            break;

        case NETACCESS_PORTMODE_SECURE:
            NETACCESS_VM_RUN_SECURE_SM(lport, state_machine);
            break;

        case NETACCESS_PORTMODE_MAC_ADDRESS_OR_USER_LOGIN_SECURE:
            NETACCESS_VM_RUN_MAC_ADDRESS_OR_USER_LOGIN_SECURE_SM(lport, state_machine);
            break;

        case NETACCESS_PORTMODE_MAC_AUTHENTICATION:
            NETACCESS_VM_RUN_MAC_AUTHENTICATION_SM(lport, state_machine);
            break;

        case NETACCESS_PORTMODE_PORT_SECURITY:
            NETACCESS_VM_RUN_PORT_SECURITY_SM(lport, state_machine);
            break;

        case NETACCESS_PORTMODE_DOT1X:
            NETACCESS_VM_RUN_DOT1X_SM(lport, state_machine);
            break;

        case NETACCESS_PORTMODE_USER_LOGIN_WITH_OUI: /* not support this mode */
        default:
            NETACCESS_VM_DEBUG_PRINT_ERROR("Wrong port mode(%lu)", state_machine->running_port_mode);
            return FALSE;
    }

    return TRUE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_TranPortModeChangeSM
 * ---------------------------------------------------------------------
 * PURPOSE: do port mode change state machine transit
 * INPUT:  none.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_TranPortModeChangeSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    UI32_T                              debug_flag;
    NETACCESS_OM_StateMachineStatus_T   pre_state;

    if (NULL == state_machine)
        return FALSE;

    debug_flag = NETACCESS_OM_GetDebugFlag();
    pre_state = state_machine->port_mode_change_sm.running_state;

    switch (state_machine->port_mode_change_sm.running_state)
    {
        case NETACCESS_STATE_ENTER_SECURE_PORT_MODE:
            state_machine->port_mode_change_sm.running_state = NETACCESS_STATE_SECURE_PORT_MODE;
            break;

        case NETACCESS_STATE_SECURE_PORT_MODE:
            if (state_machine->new_port_mode == state_machine->running_port_mode)
                return TRUE;

            /* port mode change */
            state_machine->port_mode_change_sm.running_state = NETACCESS_STATE_EXIT_SECURE_PORT_MODE;
            break;

        case NETACCESS_STATE_EXIT_SECURE_PORT_MODE:
            state_machine->port_mode_change_sm.running_state = NETACCESS_STATE_ENTER_SECURE_PORT_MODE;
            break;

        default:
            /* system initial state */
            state_machine->port_mode_change_sm.running_state = NETACCESS_STATE_ENTER_SECURE_PORT_MODE;
            break;
    }

    if (NETACCESS_OM_DEBUG_VM_IFO & debug_flag)
    {
        printf("\r\n[%s] port(%lu) from", __FUNCTION__, lport);
        NETACCESS_VM_LocalPrintState(pre_state);
        printf("state to");
        NETACCESS_VM_LocalPrintState(state_machine->port_mode_change_sm.running_state);
        printf("state");
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ExecPortModeChangeSM
 * ---------------------------------------------------------------------
 * PURPOSE: execute port mode change state machine
 * INPUT:  none.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_ExecPortModeChangeSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    UI32_T      debug_flag = 0;

    if (NULL == state_machine)
        return FALSE;

    debug_flag = NETACCESS_OM_GetDebugFlag();

    switch (state_machine->port_mode_change_sm.running_state)
    {
        case NETACCESS_STATE_ENTER_SECURE_PORT_MODE:
            if (FALSE == NETACCESS_VM_EnterSecurePortMode(lport, state_machine))
                return FALSE;

            /* UCT */
            NETACCESS_VM_RUN_PORT_MODE_CHANGE_SM(lport, state_machine);
            break;

        case NETACCESS_STATE_SECURE_PORT_MODE:
            state_machine->running_port_mode = state_machine->new_port_mode;

            /* Initial port security state machine om */
            state_machine->port_security_sm.running_state = NETACCESS_STATE_SYSTEM_INIT;
            state_machine->port_security_sm.new_mac_msg = NULL;
            state_machine->port_security_sm.radius_msg = NULL;
            state_machine->port_security_sm.dot1x_msg = NULL;

            memset(&state_machine->port_security_sm.event_bitmap, 0, sizeof(NETACCESS_OM_StateMachineEvent_T));
            memset(state_machine->port_security_sm.authenticating_mac, 0, SYS_ADPT_MAC_ADDR_LEN);
            state_machine->port_security_sm.src_vid = 0;

            if (FALSE == NETACCESS_VM_RunSecurePortModeSM(lport, state_machine))
            {
                if (NULL != state_machine->port_security_sm.cookie)
                {
                    NETACCESS_VM_AuthResult_T auth_result;

                    auth_result.result = SYS_CALLBACK_MGR_AUTH_UNAUTHENTICATED;
                    auth_result.vid = 0;

                    NETACCESS_VM_LocalPassCookieToNextCSC(lport,
                                                          state_machine,
                                                          &auth_result);
                }
                return FALSE;
            }
            break;

        case NETACCESS_STATE_EXIT_SECURE_PORT_MODE:
            if (FALSE == NETACCESS_VM_ExitSecurePortMode(lport, state_machine))
                return FALSE;

            /* UCT */
            NETACCESS_VM_RUN_PORT_MODE_CHANGE_SM(lport, state_machine);
            break;

        default:
            if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            {
                printf("\r\n[%s] bad running state", __FUNCTION__);
            }
            return FALSE;
    }

    return NETACCESS_OM_SetPortStateMachine(lport, state_machine);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_TranNoRestrictionSM
 * ---------------------------------------------------------------------
 * PURPOSE: do noRestriction state machine transit
 * INPUT:  none.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_TranNoRestrictionSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    switch (state_machine->port_security_sm.running_state)
    {
        case NETACCESS_STATE_INIT:
            state_machine->port_security_sm.running_state = NETACCESS_STATE_IDLE;
            break;

        case NETACCESS_STATE_IDLE:
            /* no where to go */
            break;

        default:
            /* system initial state */
            state_machine->port_security_sm.running_state = NETACCESS_STATE_INIT;
            break;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ExecNoRestrictionSM
 * ---------------------------------------------------------------------
 * PURPOSE: execute noRestriction state machine
 * INPUT:  none.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_ExecNoRestrictionSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    switch (state_machine->port_security_sm.running_state)
    {
        case NETACCESS_STATE_INIT:
            state_machine->running_port_mode = state_machine->new_port_mode;

            /* UCT */
            NETACCESS_VM_RUN_NO_RESTRICTION_SM(lport, state_machine);
            break;

        case NETACCESS_STATE_IDLE:
            NETACCESS_VM_LocalEnterIdelState(lport, state_machine);

            /* dot1x in force-auth mode needs to receive EAP packets (ex: EAPOL Start), too.
             */
            if (1 == state_machine->port_security_sm.event_bitmap.eap_packet)
            {
                state_machine->port_security_sm.event_bitmap.eap_packet = 0;

                if (FALSE == NETACCESS_VM_LocalForwardEapPacketToDot1x(lport, state_machine))
                {
                    return FALSE;
                }
            }

            break;

        default:
            return FALSE;
    }

    return NETACCESS_OM_SetPortStateMachine(lport, state_machine);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_TranMacAuthenticationSM
 * ---------------------------------------------------------------------
 * PURPOSE: do macAuthentication state machine transit
 * INPUT:  none.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_TranMacAuthenticationSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    UI32_T                              debug_flag;
    NETACCESS_OM_StateMachineStatus_T   pre_state;

    debug_flag = NETACCESS_OM_GetDebugFlag();
    pre_state = state_machine->port_security_sm.running_state;

    switch (state_machine->port_security_sm.running_state)
    {
        case NETACCESS_STATE_INIT:
            state_machine->port_security_sm.running_state = NETACCESS_STATE_IDLE;
            break;

        case NETACCESS_STATE_IDLE:
            if ((1 == state_machine->port_security_sm.event_bitmap.new_mac) ||
                (1 == state_machine->port_security_sm.event_bitmap.reauth))
            {
                state_machine->port_security_sm.running_state = NETACCESS_STATE_AUTHENTICATING;
                break;
            }

            /* nothing happened, no tansition */
            return TRUE;

        case NETACCESS_STATE_AUTHENTICATING:
            if (1 == state_machine->port_security_sm.event_bitmap.rada_success)
            {
                state_machine->port_security_sm.running_state = NETACCESS_STATE_SUCCEEDED;
                break;
            }
            else if (1 == state_machine->port_security_sm.event_bitmap.rada_fail)
            {
                state_machine->port_security_sm.running_state = NETACCESS_STATE_FAILED;
                break;
            }

            /* nothing happened, no tansition */
            return TRUE;

        case NETACCESS_STATE_SUCCEEDED:
            state_machine->port_security_sm.running_state = NETACCESS_STATE_IDLE;
            break;

        case NETACCESS_STATE_FAILED:
            state_machine->port_security_sm.running_state = NETACCESS_STATE_IDLE;
            break;

        default:
            /* system initial state */
            state_machine->port_security_sm.running_state = NETACCESS_STATE_INIT;
            break;
    }

    if (NETACCESS_OM_DEBUG_VM_IFO & debug_flag)
    {
        printf("\r\n[%s] port(%lu) from", __FUNCTION__, lport);
        NETACCESS_VM_LocalPrintState(pre_state);
        printf("state to");
        NETACCESS_VM_LocalPrintState(state_machine->port_security_sm.running_state);
        printf("state");
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ExecMacAuthenticationSM
 * ---------------------------------------------------------------------
 * PURPOSE: execute macAuthentication state machine
 * INPUT:  none.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_ExecMacAuthenticationSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    UI32_T      debug_flag = 0;

    debug_flag = NETACCESS_OM_GetDebugFlag();

    if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
    {
        printf("\r\n[%s] execute state machine for state", __FUNCTION__);
        NETACCESS_VM_LocalPrintState(state_machine->port_security_sm.running_state);
    }

    switch (state_machine->port_security_sm.running_state)
    {
        case NETACCESS_STATE_INIT:
            state_machine->running_port_mode = state_machine->new_port_mode;

            /* UCT */
            NETACCESS_VM_RUN_MAC_AUTHENTICATION_SM(lport, state_machine);
            break;

        case NETACCESS_STATE_IDLE:
            NETACCESS_VM_LocalEnterIdelState(lport, state_machine);

            state_machine->port_security_sm.event_bitmap.new_mac = 0;
            state_machine->port_security_sm.event_bitmap.reauth = 0;
            state_machine->port_security_sm.event_bitmap.waiting_reauth_result = 0;
            state_machine->port_security_sm.event_bitmap.is_authenticating = 0;
            break;

        case NETACCESS_STATE_AUTHENTICATING:
            if (FALSE == NETACCESS_VM_LocalEnterAuthenticating(lport, state_machine))
                return FALSE;

            if (1 == state_machine->port_security_sm.event_bitmap.new_mac)
            {
                if (FALSE == NETACCESS_VM_LocalTriggerRadaDoAuthentication(lport, state_machine))
                    return FALSE;
            }
            else if (1 == state_machine->port_security_sm.event_bitmap.reauth)
            {
                if (FALSE == NETACCESS_VM_LocalTriggerRadaDoAuthentication(lport, state_machine))
                    return FALSE;
                state_machine->port_security_sm.event_bitmap.waiting_reauth_result = 1;
            }

            state_machine->port_security_sm.event_bitmap.reauth = 0;
            state_machine->port_security_sm.event_bitmap.new_mac = 0;
            state_machine->port_security_sm.event_bitmap.rada_success = 0;
            state_machine->port_security_sm.event_bitmap.rada_fail = 0;
            break;

        case NETACCESS_STATE_SUCCEEDED:

            if (FALSE == NETACCESS_VM_UserLogin(lport, state_machine))
            {
                return FALSE;
            }

            /* UCT */
            NETACCESS_VM_RUN_MAC_AUTHENTICATION_SM(lport, state_machine);
            break;

        case NETACCESS_STATE_FAILED:

            NETACCESS_VM_LocalExecMacAuthenticationIntrusionAction(lport, state_machine);

            /* UCT */
            NETACCESS_VM_RUN_MAC_AUTHENTICATION_SM(lport,state_machine);
            break;

        default:
            if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            {
                printf("\r\n[%s] bad running state", __FUNCTION__);
            }
            return FALSE;
    }

    return NETACCESS_OM_SetPortStateMachine(lport, state_machine);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETWORKACCESS_VM_TranPortSecuritySM
 * ---------------------------------------------------------------------
 * PURPOSE: do PortSecurity state machine transit
 * INPUT:  none.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_TranPortSecuritySM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{

    switch (state_machine->port_security_sm.running_state)
    {
        case NETACCESS_STATE_INIT:
            state_machine->port_security_sm.running_state = NETACCESS_STATE_IDLE;
            break;

        case NETACCESS_STATE_IDLE:
            if (1 == state_machine->port_security_sm.event_bitmap.new_mac)
            {
                if(FALSE == NETACCESS_OM_IsSecureAddressesFull(lport))
                {
                    state_machine->port_security_sm.running_state = NETACCESS_STATE_LEARNING;
                }
                else
                {
                    state_machine->port_security_sm.running_state = NETACCESS_STATE_INTRUSION_HANDLING;
                }
            }
            break;

        case NETACCESS_STATE_LEARNING:
            state_machine->port_security_sm.running_state = NETACCESS_STATE_IDLE;
            break;

        case NETACCESS_STATE_INTRUSION_HANDLING:
            state_machine->port_security_sm.running_state = NETACCESS_STATE_IDLE;
            break;

        default:
            /* system initial state */
            state_machine->port_security_sm.running_state = NETACCESS_STATE_INIT;
            break;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ExecPortSecuritySM
 * ---------------------------------------------------------------------
 * PURPOSE: execute PortSecurity state machine
 * INPUT:  none.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_ExecPortSecuritySM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{

    switch (state_machine->port_security_sm.running_state)
    {
        case NETACCESS_STATE_INIT:
            state_machine->running_port_mode = state_machine->new_port_mode;

            /* UCT */
            NETACCESS_VM_RUN_PORT_SECURITY_SM(lport, state_machine);
            break;

        case NETACCESS_STATE_IDLE:
            NETACCESS_VM_LocalEnterIdelState(lport, state_machine);

            state_machine->port_security_sm.event_bitmap.new_mac = 0;
            break;

        case NETACCESS_STATE_LEARNING:
            if (FALSE == NETACCESS_VM_LocalLearnNewMac(lport, state_machine))
            {
                return FALSE;
            }

            /* UCT */
            NETACCESS_VM_RUN_PORT_SECURITY_SM(lport, state_machine);

            break;

        case NETACCESS_STATE_INTRUSION_HANDLING:
            if (FALSE == NETACCESS_VM_LocalExecIntrusionAction(lport, state_machine))
                return FALSE;

            /* UCT */
            NETACCESS_VM_RUN_PORT_SECURITY_SM(lport, state_machine);
            break;

        default:
            return FALSE;
    }

    return NETACCESS_OM_SetPortStateMachine(lport, state_machine);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_TranDot1xSM
 * ---------------------------------------------------------------------
 * PURPOSE: do Dot1x state machine transit
 * INPUT:  none.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_TranDot1xSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    UI32_T                              dot1x_operation_mode, debug_flag;
    NETACCESS_OM_StateMachineStatus_T   pre_state;

    debug_flag = NETACCESS_OM_GetDebugFlag();
    pre_state = state_machine->port_security_sm.running_state;

    switch (state_machine->port_security_sm.running_state)
    {
        case NETACCESS_STATE_INIT:
            state_machine->port_security_sm.running_state = NETACCESS_STATE_IDLE;
            break;

        case NETACCESS_STATE_IDLE:
#if (SYS_CPNT_DOT1X == TRUE)
            DOT1X_OM_Get_PortOperationMode(lport, &dot1x_operation_mode);

            if ((1 == state_machine->port_security_sm.event_bitmap.new_mac)
                && (!DOT1X_OM_IsPortBasedMode(lport)))
            {
                state_machine->port_security_sm.running_state = NETACCESS_STATE_DISCOVERY;
                break;
            }
            /* For Multi-host mode, if this port status is authorized then pass
             * all MAC.
             */
            else if(
                       (    (1 == state_machine->port_security_sm.event_bitmap.new_mac)
                         || (1 == state_machine->port_security_sm.event_bitmap.eap_packet)
                       )
                       &&
                       (    (DOT1X_PORT_OPERATION_MODE_MULTIPASS == dot1x_operation_mode)
                         && (1 == state_machine->port_security_sm.event_bitmap.dot1x_logon)
                         && (FALSE == NETACCESS_OM_IsThisMacAuthorizedByDot1x(lport, state_machine->port_security_sm.authenticating_mac))
                       )
                   )
            {
                state_machine->port_security_sm.running_state = NETACCESS_STATE_SUCCEEDED;
                break;
            }
            else if (1 == state_machine->port_security_sm.event_bitmap.dot1x_no_eapol)
            {
                state_machine->port_security_sm.running_state = NETACCESS_STATE_PROTO_UNAWARE;
                break;
            }
            else if(
                  (    (1 == state_machine->port_security_sm.event_bitmap.eap_packet)
                    || (1 == state_machine->port_security_sm.event_bitmap.reauth)
                  )
                  &&
                  (    (DOT1X_PORT_OPERATION_MODE_ONEPASS == dot1x_operation_mode)
                    || (DOT1X_PORT_OPERATION_MODE_MACBASED == dot1x_operation_mode)
                    || (0 == state_machine->port_security_sm.event_bitmap.dot1x_logon)
                    || (TRUE == NETACCESS_OM_IsThisMacAuthorizedByDot1x(lport, state_machine->port_security_sm.authenticating_mac))
                  )
              )
            {
                state_machine->port_security_sm.running_state = NETACCESS_STATE_AUTHENTICATING;
                break;
            }
            else if (1 == state_machine->port_security_sm.event_bitmap.dot1x_logoff)
            {
                /* Logoff due to reauthentication timeout (ex: supplicant is
                 * not exist anymore)
                 */
                state_machine->port_security_sm.running_state = NETACCESS_STATE_FAILED;
                break;
            }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

            /* nothing happened, no tansition */
            return TRUE;

        case NETACCESS_STATE_DISCOVERY:
            state_machine->port_security_sm.running_state = NETACCESS_STATE_IDLE;
            break;

        case NETACCESS_STATE_AUTHENTICATING:
            if (1 == state_machine->port_security_sm.event_bitmap.dot1x_success)
            {
                state_machine->port_security_sm.running_state = NETACCESS_STATE_SUCCEEDED;
                break;
            }
            else if ((1 == state_machine->port_security_sm.event_bitmap.dot1x_fail) ||
                     (1 == state_machine->port_security_sm.event_bitmap.dot1x_logoff))
            {
                state_machine->port_security_sm.running_state = NETACCESS_STATE_FAILED;
                break;
            }
            else if (1 == state_machine->port_security_sm.event_bitmap.eap_packet)
            {
                /* no tansition */
            }
            else if (1 == state_machine->port_security_sm.event_bitmap.dot1x_no_eapol)
            {
                state_machine->port_security_sm.running_state = NETACCESS_STATE_PROTO_UNAWARE;
                break;
            }

            /* nothing happened, no tansition */
            return TRUE;

        case NETACCESS_STATE_PROTO_UNAWARE:
            state_machine->port_security_sm.running_state = NETACCESS_STATE_IDLE;
            break;

        case NETACCESS_STATE_SUCCEEDED:
            state_machine->port_security_sm.running_state = NETACCESS_STATE_IDLE;
            break;

        case NETACCESS_STATE_FAILED:
            state_machine->port_security_sm.running_state = NETACCESS_STATE_INTRUSION_HANDLING;
            break;

        case NETACCESS_STATE_INTRUSION_HANDLING:
            state_machine->port_security_sm.running_state = NETACCESS_STATE_IDLE;
            break;

        default:
            /* system initial state */
            state_machine->port_security_sm.running_state = NETACCESS_STATE_INIT;
            break;
    }

    NETACCESS_VM_DEBUG_PRINT_IMPORTANT("dot1x-sm: port(%lu) from (%s) state to (%s) state",
                                       lport,
                                       NETACCESS_VM_LocalStringState(pre_state),
                                       NETACCESS_VM_LocalStringState(
                                       state_machine->port_security_sm.running_state));

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ExecDot1xSM
 * ---------------------------------------------------------------------
 * PURPOSE: execute Dot1x state machine
 * INPUT:  none.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_ExecDot1xSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    switch (state_machine->port_security_sm.running_state)
    {
        case NETACCESS_STATE_INIT:
            state_machine->running_port_mode = state_machine->new_port_mode;
            state_machine->port_security_sm.event_bitmap.dot1x_logon = 0;

            /* UCT */
            NETACCESS_VM_RUN_DOT1X_SM(lport, state_machine);
            break;

        case NETACCESS_STATE_IDLE:
            NETACCESS_VM_LocalEnterIdelState(lport, state_machine);

            state_machine->port_security_sm.event_bitmap.new_mac = 0;
            state_machine->port_security_sm.event_bitmap.eap_packet = 0;
            state_machine->port_security_sm.event_bitmap.reauth = 0;
            state_machine->port_security_sm.event_bitmap.waiting_reauth_result = 0;
            state_machine->port_security_sm.event_bitmap.is_authenticating = 0;
            state_machine->port_security_sm.event_bitmap.dot1x_no_eapol = 0;
            state_machine->port_security_sm.event_bitmap.dot1x_logoff = 0;
            break;

        case NETACCESS_STATE_DISCOVERY:
            if (FALSE == NETACCESS_VM_LocalTriggerDot1xSendEapRequest(lport, state_machine))
                return FALSE;

            NETACCESS_VM_RUN_DOT1X_SM(lport, state_machine);
            break;

        case NETACCESS_STATE_AUTHENTICATING:
            if (FALSE == NETACCESS_VM_LocalEnterAuthenticating(lport, state_machine))
                return FALSE;

            /* the checking sequence is important */
            if (1 == state_machine->port_security_sm.event_bitmap.eap_packet)
            {
                if (1 == state_machine->port_security_sm.event_bitmap.reauth)
                    state_machine->port_security_sm.event_bitmap.waiting_reauth_result = 1;

                if (FALSE == NETACCESS_VM_LocalForwardEapPacketToDot1x(lport, state_machine))
                    return FALSE;
            }
            else if (1 == state_machine->port_security_sm.event_bitmap.reauth)
            {
                state_machine->port_security_sm.event_bitmap.waiting_reauth_result = 1;
            }

            state_machine->port_security_sm.event_bitmap.reauth = 0;
            state_machine->port_security_sm.event_bitmap.new_mac = 0;
            state_machine->port_security_sm.event_bitmap.eap_packet = 0;
            state_machine->port_security_sm.event_bitmap.dot1x_success = 0;
            state_machine->port_security_sm.event_bitmap.dot1x_fail = 0;
            state_machine->port_security_sm.event_bitmap.dot1x_logoff = 0;
            state_machine->port_security_sm.event_bitmap.dot1x_no_eapol = 0;

            break;

        case NETACCESS_STATE_PROTO_UNAWARE:

            /* join to guest VLAN if it be configured
             */
            if (    (FALSE == NETACCESS_VM_IsInRestrictedVlan(lport))
                 && (TRUE == NETACCESS_VM_LocalIsNeedToJoinGuestVlan(lport))
               )
            {
                if (    (FALSE == NETACCESS_VM_SetToGuestVlan(lport))
                    ||  (FALSE == NETACCESS_VM_LocalEnableAutoLearning(lport))
                   )
                {
                    NETACCESS_VM_NotifyDot1xManagerSetupResult(lport, state_machine, FALSE);
                    return FALSE;
                }
            }

            NETACCESS_VM_NotifyDot1xManagerSetupResult(lport, state_machine, TRUE);

            /* UCT */
            NETACCESS_VM_RUN_DOT1X_SM(lport, state_machine);
            break;

        case NETACCESS_STATE_SUCCEEDED:

            if (FALSE == NETACCESS_VM_UserLogin(lport, state_machine))
            {
                NETACCESS_VM_NotifyDot1xManagerSetupResult(lport, state_machine, FALSE);
                return FALSE;
            }

            state_machine->port_security_sm.event_bitmap.dot1x_logon = 1;

            NETACCESS_VM_NotifyDot1xManagerSetupResult(lport, state_machine, TRUE);

            /* UCT */
            NETACCESS_VM_RUN_DOT1X_SM(lport, state_machine);
            break;

        case NETACCESS_STATE_FAILED:

            if (FALSE == NETACCESS_VM_LocalEnterDot1xAuthFailedState(lport, state_machine))
            {
                return FALSE;
            }

            /* UCT */
            NETACCESS_VM_RUN_DOT1X_SM(lport, state_machine);
            break;

        case NETACCESS_STATE_INTRUSION_HANDLING:
            /* A3COM0458 Mib, userLogin(5)
               The Need To Know and Intrusion Action are ignored.
             */

            /* UCT */
            NETACCESS_VM_RUN_DOT1X_SM(lport, state_machine);
            break;

        default:
            NETACCESS_VM_DEBUG_PRINT_ERROR(
                           "Wrong running state(%d)",
                           state_machine->port_security_sm.running_state);
            return FALSE;
    }

    return NETACCESS_OM_SetPortStateMachine(lport, state_machine);
}

static BOOL_T NETACCESS_VM_UserLogin(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    /* leave guest VLAN, if an authorized ueser login
     */
    if (TRUE == NETACCESS_VM_IsInRestrictedVlan(lport))
    {
        if (    (FALSE == NETACCESS_VM_DeleteAllDynamicMacFromAmtrByPort(lport))
            ||  (FALSE == NETACCESS_VM_LeaveGuestVlanDisableAutoLearningAndTrap2Cpu(lport, state_machine))
           )
        {
            return FALSE;
        }
    }

    /* learn new mac
     */
    if (FALSE == NETACCESS_VM_LocalLearnNewMac(lport, state_machine))
    {
        return FALSE;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalIsNeedToJoinGuestVlan
 * ---------------------------------------------------------------------
 * PURPOSE  : Check the port need to join to guest VLAN
 * INPUT    : lport -- lport index
 * OUTPUT   : None.
 * RETURN   : TRUE -- need to join to guest VLAN, FALSE -- no
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalIsNeedToJoinGuestVlan(UI32_T lport)
{
    UI32_T vid;
    VLAN_OM_Vlan_Port_Info_T    port_info;

    if (FALSE == NETACCESS_OM_GetSecureGuestVlanId(lport, &vid))
    {
        return FALSE;
    }

    memset(&port_info, 0, sizeof(VLAN_OM_Vlan_Port_Info_T));
    if(FALSE == VLAN_PMGR_GetPortEntry(lport, &port_info))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR(
            "VLAN_PMGR_GetPortEntry(%lu) failed", lport);
        return FALSE;
    }

    if (    (TRUE == VLAN_OM_IsVlanExisted(vid))
        &&  (VAL_vlanPortMode_access != port_info.vlan_port_entry.vlan_port_mode)
       )
    {
        return TRUE;
    }

    return FALSE;
}

#if (SYS_CPNT_DOT1X == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalIsNeedToJoinAuthFailVlan
 * ---------------------------------------------------------------------
 * PURPOSE  : Check the port need to join to auth-fail VLAN
 * INPUT    : lport -- lport index
 * OUTPUT   : None.
 * RETURN   : TRUE -- need to join to auth-fail VLAN, FALSE -- no
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalIsNeedToJoinAuthFailVlan(UI32_T lport)
{
    UI32_T vid, action_state;
    VLAN_OM_Vlan_Port_Info_T    port_info;

    if (FALSE == NETACCESS_OM_GetSecureGuestVlanId(lport, &vid))
    {
        return FALSE;
    }

    action_state = DOT1X_OM_GetPortIntrusionActionStatus(lport);

    memset(&port_info, 0, sizeof(VLAN_OM_Vlan_Port_Info_T));
    if(FALSE == VLAN_PMGR_GetPortEntry(lport, &port_info))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR(
            "VLAN_PMGR_GetPortEntry(%lu) failed", lport);
        return FALSE;
    }

    if (    (VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan == action_state)
        &&  (TRUE == VLAN_OM_IsVlanExisted(vid))
        &&  (VAL_vlanPortMode_access != port_info.vlan_port_entry.vlan_port_mode)
        &&  (TRUE == DOT1X_OM_IsPortBasedMode(lport))
       )
    {
        return TRUE;
    }

    return FALSE;
}
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalHaveDot1xMac
 * ---------------------------------------------------------------------
 * PURPOSE  : Check there have any dot1x authorized MAC on the port.
 * INPUT    : lport.
 * OUTPUT   : None.
 * RETURN   : TRUE -- have dot1x MAC on the port, FALSE -- no
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalHaveDot1xMac(UI32_T lport)
{
    NETACCESS_OM_SecureMacEntry_T   mac_entry;

    memset(&mac_entry, 0, sizeof(mac_entry));
    mac_entry.lport = lport;

    while(NETACCESS_OM_GetNextSecureAddressEntry(&mac_entry)==TRUE)
    {
        /* check if same port
         */
        if(mac_entry.lport != lport)
        {
            break;
        }

        /* check the mac is authorized by dot1x
         */
        if (1 == mac_entry.mac_flag.authorized_by_dot1x)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_IsInRestrictedVlan
 * ---------------------------------------------------------------------
 * PURPOSE  : Check the port is in restricted VLAN
 * INPUT    : lport -- lport index
 * OUTPUT   : None.
 * RETURN   : TRUE -- in restricted VLAN, FALSE -- no
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_IsInRestrictedVlan(UI32_T lport)
{
    BOOL_T is_join_restricted_vlan;

    if (FALSE == NETACCESS_OM_GetJoinRestrictedVlanStatus(lport, &is_join_restricted_vlan))
    {
        return FALSE;
    }

    return (is_join_restricted_vlan == TRUE) ? TRUE : FALSE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_AddToGuestVlan
 * ---------------------------------------------------------------------
 * PURPOSE  : Add the port to restricted VLAN
 * INPUT    : lport -- lport index
 * OUTPUT   : None.
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_SetToGuestVlan(UI32_T lport)
{
    UI32_T              vid;
    char guest_vlan_str[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1];
    UI8_T applied_md5[16];

#if 0
    Global_Params *pae_state;

    pae_state=DOT1X_OM_GetStateMachineWorkingArea(lport);
    if(pae_state == NULL)
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR(
        "DOT1X_OM_GetStateMachineWorkingArea(%lu) fail", lport);
        return FALSE;
    }
#endif

    if (FALSE == NETACCESS_OM_GetSecureGuestVlanId(lport, &vid))
    {
        return FALSE;
    }

    NETACCESS_VM_LocalLeaveAuthorizedVlan(lport);

    sprintf(guest_vlan_str, "%lup", vid);
    if (!PORTAUTHSRVC_MGR_Vlan_SetToOper(lport, guest_vlan_str))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("PORTAUTHSRVC_MGR_Vlan_SetToOper failed\r\n"
                                       "  lport(%lu), VLAN(%s)",
                                       lport, guest_vlan_str);
        return FALSE;
    }

    PORTAUTHSRVC_MGR_Vlan_StringToMd5(guest_vlan_str, applied_md5);
    NETACCESS_OM_SetDynamicVlanMd5(lport, applied_md5);
    NETACCESS_OM_SetJoinRestrictedVlanStatus(lport, TRUE);

#if 0
    /* trigger the Authenticator PAE state machine
     */
    DOT1X_OM_EnterCriticalRegion();
    DOT1X_VM_Do_Authenticator(pae_state, DOT1X_NETACCESS_CONTROL);
    DOT1X_OM_LeaveCriticalRegion();
#endif
    return TRUE;
}

BOOL_T NETACCESS_VM_LeaveGuestVlan(UI32_T lport)
{
    if (NETACCESS_VM_IsInRestrictedVlan(lport))
    {
        NETACCESS_OM_StateMachine_T netaccess_sm;

        if (FALSE == NETACCESS_VM_DeleteAllDynamicMacFromAmtrByPort(lport))
        {
            NETACCESS_VM_DEBUG_PRINT_ERROR("Delete dynamic MAC failed");
            return FALSE;
        }

        NETACCESS_OM_GetPortStateMachine(lport, &netaccess_sm);
        if (FALSE == NETACCESS_VM_LeaveGuestVlanDisableAutoLearningAndTrap2Cpu(lport, &netaccess_sm))
        {
            NETACCESS_VM_DEBUG_PRINT_ERROR("Reset port status failed");
            return FALSE;
        }

#if (SYS_CPNT_DOT1X == TRUE)
        DOT1X_VM_SendEvent(lport, DOT1X_SM_AUTH_EAPLOGOFF_EV);
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */
    }

    return TRUE;
}

#if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_SetDriverForRecievingEapPacket
 * ---------------------------------------------------------------------
 * PURPOSE: The action to driver when receive an EAP packet.
 * INPUT  : dot1x_sys_ctrl  -- global status of 802.1X
 *          eapol_pass_thru -- status of EAPOL frames pass-through
 * OUTPUT : None.
 * RETURN : TRUE  -- succeeded
 *          FALSE -- failed
 * NOTES :
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_SetDriverForRecievingEapPacket(UI32_T dot1x_sys_ctrl, DOT1X_OM_EapolPassThru_T eapol_pass_thru)
{
    enum
    {
        PASS_THROU_EAP_PKT_AND_NO_TRAP_TO_CPU = TRUE,
        TRAP_EAP_PKT_TO_CPU = FALSE,
    };

    BOOL_T status = TRAP_EAP_PKT_TO_CPU;

    if ((VAL_dot1xPaeSystemAuthControl_disabled == dot1x_sys_ctrl)
        && (DOT1X_OM_EAPOL_PASS_THRU_ENABLED == eapol_pass_thru))
    {
        status = PASS_THROU_EAP_PKT_AND_NO_TRAP_TO_CPU;
    }

    if (FALSE == SWCTRL_SetEapolFramePassThrough(status))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR(
            "SWCTRL_SetEapolFramesPassThrough(%s) failed",
            (status == TRUE) ? "TRUE" : "FALSE");
        return FALSE;
    }

    return TRUE;
}
#endif /* #if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE) */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_TranSecureSM
 * ---------------------------------------------------------------------
 * PURPOSE: do secure state machine transit
 * INPUT:  none.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_TranSecureSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    UI32_T                              debug_flag;
    NETACCESS_OM_StateMachineStatus_T   pre_state;

    debug_flag = NETACCESS_OM_GetDebugFlag();
    pre_state = state_machine->port_security_sm.running_state;

    switch (state_machine->port_security_sm.running_state)
    {
        case NETACCESS_STATE_INIT:
            state_machine->port_security_sm.running_state = NETACCESS_STATE_IDLE;
            break;

        case NETACCESS_STATE_IDLE:
            /* dot1x in force-unauth mode needs to receive EAP packets (ex: EAPOL Start), too.
             */
            if (1 == state_machine->port_security_sm.event_bitmap.eap_packet)
            {
                state_machine->port_security_sm.event_bitmap.eap_packet = 0;

                if (FALSE == NETACCESS_VM_LocalForwardEapPacketToDot1x(lport, state_machine))
                {
                    return FALSE;
                }
            }

            if (1 == state_machine->port_security_sm.event_bitmap.new_mac)
            {
                state_machine->port_security_sm.running_state = NETACCESS_STATE_INTRUSION_HANDLING;
                break;
            }

            /* nothing happened, no tansition */
            return TRUE;

        case NETACCESS_STATE_INTRUSION_HANDLING:
            state_machine->port_security_sm.running_state = NETACCESS_STATE_IDLE;
            break;

        default:
            /* system initial state */
            state_machine->port_security_sm.running_state = NETACCESS_STATE_INIT;
            break;
    }

    if (NETACCESS_OM_DEBUG_VM_IFO & debug_flag)
    {
        printf("\r\n[%s] port(%lu) from", __FUNCTION__, lport);
        NETACCESS_VM_LocalPrintState(pre_state);
        printf("state to");
        NETACCESS_VM_LocalPrintState(state_machine->port_security_sm.running_state);
        printf("state");
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ExecSecureSM
 * ---------------------------------------------------------------------
 * PURPOSE: execute secure state machine
 * INPUT:  none.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_ExecSecureSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    UI32_T      debug_flag = 0;

    debug_flag = NETACCESS_OM_GetDebugFlag();

    switch (state_machine->port_security_sm.running_state)
    {
        case NETACCESS_STATE_INIT:
            state_machine->running_port_mode = state_machine->new_port_mode;

            /* UCT */
            NETACCESS_VM_RUN_SECURE_SM(lport, state_machine);
            break;

        case NETACCESS_STATE_IDLE:
            NETACCESS_VM_LocalEnterIdelState(lport, state_machine);

            state_machine->port_security_sm.event_bitmap.new_mac = 0;
            break;

        case NETACCESS_STATE_INTRUSION_HANDLING:
            if (FALSE == NETACCESS_VM_LocalExecIntrusionAction(lport, state_machine))
                return FALSE;

            /* UCT */
            NETACCESS_VM_RUN_SECURE_SM(lport, state_machine);
            break;

        default:
            if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            {
                printf("\r\n[%s] bad running state", __FUNCTION__);
            }
            return FALSE;
    }

    return NETACCESS_OM_SetPortStateMachine(lport, state_machine);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_TranMacAddressOrUserLoginSecureSM
 * ---------------------------------------------------------------------
 * PURPOSE: do macAddressOrUserLoginSecure state machine transit
 * INPUT:  none.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_TranMacAddressOrUserLoginSecureSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    UI32_T                              debug_flag;
    NETACCESS_OM_StateMachineStatus_T   pre_state;

    debug_flag = NETACCESS_OM_GetDebugFlag();
    pre_state = state_machine->port_security_sm.running_state;

    switch (state_machine->port_security_sm.running_state)
    {
        case NETACCESS_STATE_INIT:
            state_machine->port_security_sm.running_state = NETACCESS_STATE_IDLE;
            break;

        case NETACCESS_STATE_IDLE:
            if (1 == state_machine->port_security_sm.event_bitmap.eap_packet)
            {
                state_machine->port_security_sm.running_state = NETACCESS_STATE_DOT1X_AUTHENTICATING;
                break;
            }
            else if (1 == state_machine->port_security_sm.event_bitmap.new_mac)
            {
                state_machine->port_security_sm.running_state = NETACCESS_STATE_RADA_AUTHENTICATING;
                break;
            }
            else if (1 == state_machine->port_security_sm.event_bitmap.reauth)
            {
                if (TRUE == NETACCESS_OM_IsThisMacLearnedFromEapPacket(lport, state_machine->port_security_sm.authenticating_mac))
                    state_machine->port_security_sm.running_state = NETACCESS_STATE_DOT1X_AUTHENTICATING;
                else
                    state_machine->port_security_sm.running_state = NETACCESS_STATE_RADA_AUTHENTICATING;
                break;
            }

            /* nothing happened, no tansition */
            return TRUE;

        case NETACCESS_STATE_DOT1X_AUTHENTICATING:
            if (1 == state_machine->port_security_sm.event_bitmap.dot1x_success)
            {
                state_machine->port_security_sm.running_state = NETACCESS_STATE_SUCCEEDED;
                break;
            }
            else if ((1 == state_machine->port_security_sm.event_bitmap.dot1x_fail) ||
                     (1 == state_machine->port_security_sm.event_bitmap.dot1x_logoff))
            {
                state_machine->port_security_sm.running_state = NETACCESS_STATE_DOT1X_FAILED;
                break;
            }

            /* nothing happened, no tansition */
            return TRUE;

        case NETACCESS_STATE_RADA_AUTHENTICATING:
            if (1 == state_machine->port_security_sm.event_bitmap.rada_success)
            {
                state_machine->port_security_sm.running_state = NETACCESS_STATE_SUCCEEDED;
                break;
            }
            else if (1 == state_machine->port_security_sm.event_bitmap.rada_fail)
            {
                state_machine->port_security_sm.running_state = NETACCESS_STATE_RADA_FAILED;
                break;
            }

            /* nothing happened, no tansition */
            return TRUE;

        case NETACCESS_STATE_SUCCEEDED:
            state_machine->port_security_sm.running_state = NETACCESS_STATE_IDLE;
            break;

        case NETACCESS_STATE_DOT1X_FAILED:
            state_machine->port_security_sm.running_state = NETACCESS_STATE_RADA_AUTHENTICATING;
            break;

        case NETACCESS_STATE_RADA_FAILED:
            state_machine->port_security_sm.running_state = NETACCESS_STATE_INTRUSION_HANDLING;
            break;

        case NETACCESS_STATE_INTRUSION_HANDLING:
            state_machine->port_security_sm.running_state = NETACCESS_STATE_IDLE;
            break;

        default:
            /* system initial state */
            state_machine->port_security_sm.running_state = NETACCESS_STATE_INIT;
            break;
    }

    if (NETACCESS_OM_DEBUG_VM_IFO & debug_flag)
    {
        printf("\r\n[%s] port(%lu) from", __FUNCTION__, lport);
        NETACCESS_VM_LocalPrintState(pre_state);
        printf("state to");
        NETACCESS_VM_LocalPrintState(state_machine->port_security_sm.running_state);
        printf("state");
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ExecMacAddressOrUserLoginSecureSM
 * ---------------------------------------------------------------------
 * PURPOSE: execute macAddressOrUserLoginSecure state machine
 * INPUT:  none.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_ExecMacAddressOrUserLoginSecureSM(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    UI32_T      debug_flag = 0;

    debug_flag = NETACCESS_OM_GetDebugFlag();

    switch (state_machine->port_security_sm.running_state)
    {
        case NETACCESS_STATE_INIT:
            state_machine->running_port_mode = state_machine->new_port_mode;
            state_machine->port_security_sm.event_bitmap.dot1x_logon = 0;

            /* UCT */
            NETACCESS_VM_RUN_MAC_ADDRESS_OR_USER_LOGIN_SECURE_SM(lport, state_machine);
            break;

        case NETACCESS_STATE_IDLE:
            NETACCESS_VM_LocalEnterIdelState(lport, state_machine);

            state_machine->port_security_sm.event_bitmap.new_mac = 0;
            state_machine->port_security_sm.event_bitmap.eap_packet = 0;
            state_machine->port_security_sm.event_bitmap.reauth = 0;
            state_machine->port_security_sm.event_bitmap.waiting_reauth_result = 0;
            state_machine->port_security_sm.event_bitmap.is_authenticating = 0;
            break;

        case NETACCESS_STATE_DOT1X_AUTHENTICATING:
            if (FALSE == NETACCESS_VM_LocalEnterAuthenticating(lport, state_machine))
                return FALSE;

            /* To process dot1x authenticaing state
             * as same as in NETACCESS_VM_ExecDot1xSM
             */
            if (1 == state_machine->port_security_sm.event_bitmap.eap_packet)
            {
                if (1 == state_machine->port_security_sm.event_bitmap.reauth)
                    state_machine->port_security_sm.event_bitmap.waiting_reauth_result = 1;

                if (FALSE == NETACCESS_VM_LocalForwardEapPacketToDot1x(lport, state_machine))
                    return FALSE;
            }
            else if (1 == state_machine->port_security_sm.event_bitmap.reauth)
            {
                state_machine->port_security_sm.event_bitmap.waiting_reauth_result = 1;
            }

            state_machine->port_security_sm.event_bitmap.reauth = 0;
            state_machine->port_security_sm.event_bitmap.eap_packet = 0;
            state_machine->port_security_sm.event_bitmap.dot1x_success = 0;
            state_machine->port_security_sm.event_bitmap.dot1x_fail = 0;
            state_machine->port_security_sm.event_bitmap.dot1x_logoff = 0;
            break;

        case NETACCESS_STATE_RADA_AUTHENTICATING:
            if (FALSE == NETACCESS_VM_LocalEnterAuthenticating(lport, state_machine))
                return FALSE;

            if ((1 == state_machine->port_security_sm.event_bitmap.new_mac) ||
                (1 == state_machine->port_security_sm.event_bitmap.dot1x_fail) ||
                (1 == state_machine->port_security_sm.event_bitmap.dot1x_logoff))
            {
                if (FALSE == NETACCESS_VM_LocalTriggerRadaDoAuthentication(lport, state_machine))
                    return FALSE;
            }
            else if (1 == state_machine->port_security_sm.event_bitmap.reauth)
            {
                if (FALSE == NETACCESS_VM_LocalTriggerRadaDoAuthentication(lport, state_machine))
                    return FALSE;
                state_machine->port_security_sm.event_bitmap.waiting_reauth_result = 1;
            }

            state_machine->port_security_sm.event_bitmap.reauth = 0;
            state_machine->port_security_sm.event_bitmap.new_mac = 0;
            state_machine->port_security_sm.event_bitmap.dot1x_fail = 0;
            state_machine->port_security_sm.event_bitmap.rada_success = 0;
            state_machine->port_security_sm.event_bitmap.rada_fail = 0;
            state_machine->port_security_sm.event_bitmap.dot1x_logoff = 0;
            break;

        case NETACCESS_STATE_SUCCEEDED:

            if (FALSE == NETACCESS_VM_UserLogin(lport, state_machine))
            {
                NETACCESS_VM_NotifyDot1xManagerSetupResult(lport, state_machine, FALSE);
                return FALSE;
            }

            NETACCESS_VM_NotifyDot1xManagerSetupResult(lport, state_machine, TRUE);

            /* UCT */
            NETACCESS_VM_RUN_MAC_ADDRESS_OR_USER_LOGIN_SECURE_SM(lport, state_machine);
            break;

        case NETACCESS_STATE_DOT1X_FAILED:

            if (FALSE == NETACCESS_VM_LocalEnterDot1xAuthFailedState(lport, state_machine))
            {
                return FALSE;
            }

            /* UCT */
            NETACCESS_VM_RUN_MAC_ADDRESS_OR_USER_LOGIN_SECURE_SM(lport, state_machine);
            break;

        case NETACCESS_STATE_RADA_FAILED:
#if 1
            if (FALSE == NETACCESS_VM_LocalExecMacAuthenticationIntrusionAction(lport, state_machine))
                return FALSE;
#else
            if (1 == state_machine->port_security_sm.event_bitmap.waiting_reauth_result) /* reauthentication failed */
            {
                /* unauthorized mac and re-apply VLAN & QoS */
                if (FALSE == NETACCESS_VM_LocalUnauthorizeExistedMac(lport, state_machine))
                    return FALSE;
            }
            else /* not reauthentication failed */
            {
                /* learn new mac. if full, replace oldest one */
                if (FALSE == NETACCESS_VM_LocalLearnNewMac(lport, state_machine, FALSE))
                    return FALSE;
            }
#endif

            /* UCT */
            NETACCESS_VM_RUN_MAC_ADDRESS_OR_USER_LOGIN_SECURE_SM(lport, state_machine);
            break;

        case NETACCESS_STATE_INTRUSION_HANDLING:
            if (FALSE == NETACCESS_VM_LocalExecIntrusionAction(lport, state_machine))
                return FALSE;

            /* UCT */
            NETACCESS_VM_RUN_MAC_ADDRESS_OR_USER_LOGIN_SECURE_SM(lport, state_machine);
            break;

        default:
            if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            {
                printf("\r\n[%s] bad running state", __FUNCTION__);
            }
            return FALSE;
    }

    return NETACCESS_OM_SetPortStateMachine(lport, state_machine);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_LocalSetupAmtrPortInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : setup amtr portinfo to learn the new mac correctly.
 * INPUT    : lport, runn_port_mode
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTE     : none
 *-------------------------------------------------------------------------*/
static BOOL_T NETACCESS_VM_LocalSetupAmtrPortInfo(
    UI32_T lport, UI32_T runn_port_mode)
{
    AMTR_MGR_PortInfo_T     amtr_port_info;

    NETACCESS_VM_DEBUG_PRINT_TRACE("lport=%ld, runn_mode=%ld", lport, runn_port_mode);

    if(!AMTR_OM_GetPortInfo(lport, &amtr_port_info))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("AMTR_OM_GetPortInfo(lport=%lu)", lport);
        return FALSE;
    }

    switch (runn_port_mode)
    {
    case NETACCESS_PORTMODE_DOT1X:
        amtr_port_info.learn_with_count = 0;
        amtr_port_info.protocol     = AMTR_MGR_PROTOCOL_DOT1X;
        /* amtr_port_info.life_time    = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET; */
        break;

    case NETACCESS_PORTMODE_MAC_AUTHENTICATION:
        amtr_port_info.learn_with_count = 0;
        amtr_port_info.protocol     = AMTR_MGR_PROTOCOL_MACAUTH;
        /* amtr_port_info.life_time    = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT; */
        break;

    case NETACCESS_PORTMODE_MAC_ADDRESS_OR_USER_LOGIN_SECURE:
        amtr_port_info.learn_with_count = 0;
        amtr_port_info.protocol     = AMTR_MGR_PROTOCOL_MACAUTH_OR_DOT1X;
        /* amtr_port_info.life_time    = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT; */
        break;

    case NETACCESS_PORTMODE_SECURE:
        amtr_port_info.learn_with_count = 0;
        amtr_port_info.protocol     = AMTR_MGR_PROTOCOL_DOT1X;
        break;

    case NETACCESS_PORTMODE_PORT_SECURITY:

      /* For both 'port security' and 'port security max-mac-count' will call this function,
          but 'port security max-mac-count' should not set port security status at all.
          The 'psec_mgr' will set amtr_port_info for port security status later.
       */
#if 0
        amtr_port_info.protocol     = AMTR_MGR_PROTOCOL_PSEC;
        amtr_port_info.life_time    = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET;
#endif
        break;

    case NETACCESS_PORTMODE_NO_RESTRICTIONS:
        amtr_port_info.learn_with_count = SYS_DFLT_PORT_SECURITY_MAX_MAC_COUNT;
        amtr_port_info.protocol         = AMTR_MGR_PROTOCOL_NORMAL;
        amtr_port_info.life_time        = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT;
        break;

    /* not sure how to set-up the amtr for following modes.
     */
    case NETACCESS_PORTMODE_CONTINUOS_LEARNING:
    case NETACCESS_PORTMODE_AUTO_LEARN:
    case NETACCESS_PORTMODE_USER_LOGIN:
    case NETACCESS_PORTMODE_USER_LOGIN_SECURE:
    case NETACCESS_PORTMODE_USER_LOGIN_WITH_OUI:
    case NETACCESS_PORTMODE_MAC_ADDRESS_WITH_RADIUS:
    case NETACCESS_PORTMODE_MAC_ADDRESS_ELSE_USER_LOGIN_SECURE:
    default:
        NETACCESS_VM_DEBUG_PRINT_ERROR("Wrong port mode(%lu)", runn_port_mode);
        return FALSE;
    }

    NETACCESS_VM_DEBUG_PRINT_TRACE("larn_count=%lu, protocol=%d, life_time=%d",
                                   amtr_port_info.learn_with_count,
                                   amtr_port_info.protocol,
                                   amtr_port_info.life_time);

    return AMTR_MGR_SetPortInfo(lport, &amtr_port_info);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_LocalDisableAutoLearningAndTrap2CPU
 *-------------------------------------------------------------------------
 * PURPOSE  : disable auto learning and set chip trap to CPU
 * INPUT    : lport, runn_port_mode
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTE     : none
 *-------------------------------------------------------------------------*/
static BOOL_T NETACCESS_VM_LocalDisableAutoLearningAndTrap2CPU(
    UI32_T lport,
    UI32_T runn_port_mode
    )
{
/*
EPR:       ES3628BT-FLF-ZZ-01013
Problem:   Port Security: Can't disable port security.
rootcasue: The port security status is save in 'port_info.port_security_enabled_by_who' and
        'port_info.port_security_status' of 'swctrl' by function 'SWCTRL_SetPortSecurityStatus'.
        The 'port_info.port_security_enabled_by_who'
        should be 'SWCTRL_PORT_SECURITY_ENABLED_BY_PSEC' when port security enable
        and be 'SWCTRL_PORT_SECURITY_ENABLED_BY_NONE' when port security disabled.
        But the 'netaccess' module will set 'port_info.port_security_enabled_by_who' as
        'SWCTRL_PORT_SECURITY_ENABLED_BY_NETACCESS' when enable port security.
        It makes system misunderstand the port security is enable or not. So the port security
        can not be disabled correctly.
sloution:  return true when runn_port_mode is port security in function
        'NETACCESS_VM_LocalDisableAutoLearningAndTrap2CPU' to avoid
        set 'port_info.port_security_enabled_by_who' as 'SWCTRL_PORT_SECURITY_ENABLED_BY_NETACCESS'
        when enable port security.(as sh_wang's suggestion)
File:      netaccess_vm.c
*/
    if (runn_port_mode == NETACCESS_PORTMODE_PORT_SECURITY)
    {
        return TRUE;
    }

    /* need to setup amtr for mac learning issue
     *   for linux platform (sw learn).
     */
    if (FALSE == NETACCESS_VM_LocalSetupAmtrPortInfo(lport, runn_port_mode))
    {
        return FALSE;
    }

    /* disable auto learning
     */
    if (FALSE == SWCTRL_SetPortSecurityStatus(lport, VAL_portSecPortStatus_enabled, SWCTRL_PORT_SECURITY_ENABLED_BY_NETACCESS))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("SWCTRL_SetPortSecurityStatus(%lu, enabled) failed", lport);
        return FALSE;
    }

    /* let intruction MAC trap to CPU
     */
    if (FALSE == SWCTRL_SetPortSecurityActionStatus (lport, VAL_portSecAction_trap))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("SWCTRL_SetPortSecurityActionStatus(%lu, trap) failed", lport);
        return FALSE;
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_LocalEnableAutoLearning
 *-------------------------------------------------------------------------
 * PURPOSE  : enable auto learning
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTE     : none
 *-------------------------------------------------------------------------*/
static BOOL_T NETACCESS_VM_LocalEnableAutoLearning(UI32_T lport)
{
    /* need to setup amtr for mac learning issue
     *   for linux platform (sw learn).
     */
    if (FALSE == NETACCESS_VM_LocalSetupAmtrPortInfo(lport, NETACCESS_PORTMODE_NO_RESTRICTIONS))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR(
            "NETACCESS_VM_LocalSetupAmtrPortInfo(%d) failed", NETACCESS_PORTMODE_NO_RESTRICTIONS);
        return FALSE;
    }

    /* enable auto learning
     */
    if (FALSE == SWCTRL_SetPortSecurityStatus(lport, VAL_portSecPortStatus_disabled, SWCTRL_PORT_SECURITY_ENABLED_BY_NONE))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR(
            "SWCTRL_SetPortSecurityStatus(%lu, disable) failed", lport);
        return FALSE;
    }

    /* Drop intruction MAC
     */
    if (FALSE == SWCTRL_SetPortSecurityActionStatus (lport, VAL_portSecAction_none))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("SWCTRL_SetPortSecurityActionStatus(none) failed");
        return FALSE;
    }

    return TRUE;
}

static BOOL_T NETACCESS_VM_LocalIsEmptyStr(const char *str)
{
    const char *p = str;

    for (;*p; p++)
    {
        if (*p != ' ' && *p != '\t')
            return FALSE;
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_LocalSetAutoVlan
 *-------------------------------------------------------------------------
 * PURPOSE  : correct vlan obsolete issue
 * INPUT    : lport, state_machine
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTE     : 1,when vlan profile has been obsoleted, must remove mac
 *            2,change port VLAN member
 *-------------------------------------------------------------------------*/
static BOOL_T NETACCESS_VM_LocalSetToAutoVlan(UI32_T lport, const char *str)
{
    BOOL_T enabled;

    if (FALSE == NETACCESS_OM_GetDynamicVlanStatus(lport, &enabled))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("Wrong lport(%lu)", lport);
        return FALSE;
    }

    if (!enabled)
    {
        return TRUE;
    }

    {
        UI8_T applied[16] = {0};
        UI8_T returned[16] = {0};

        NETACCESS_OM_GetDynamicVlanMd5(lport, applied);
        PORTAUTHSRVC_MGR_Vlan_StringToMd5(str, returned);

        if (memcmp(applied, returned, sizeof(applied)) != 0)
        {
            NETACCESS_VM_LocalLeaveAuthorizedVlan(lport);

            if (!NETACCESS_VM_LocalIsEmptyStr(str))
            {
                if (!PORTAUTHSRVC_MGR_Vlan_SetToOper(lport, str))
                {
                    NETACCESS_VM_DEBUG_PRINT_ERROR("PORTAUTHSRVC_MGR_Vlan_SetToOper failed");
                    return FALSE;
                }
            }

            NETACCESS_OM_SetDynamicVlanMd5(lport, returned);
        }
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LeaveGuestVlanDisableAutoLearningAndTrap2Cpu
 * ---------------------------------------------------------------------
 * PURPOSE  : Leave from guest VLAN, disable auto learning, and enable
 *            trap packet to CPU.
 * INPUT    : lport -- lport index
 * OUTPUT   : None
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : This function also clear the join to auto VLAN flag.
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LeaveGuestVlanDisableAutoLearningAndTrap2Cpu(
    UI32_T lport,
    NETACCESS_OM_StateMachine_T *sm_p
    )
{
    NETACCESS_VM_DEBUG_PRINT_IMPORTANT("Leave guest VLAN on port(%ld)", lport);
    if (   (FALSE == NETACCESS_VM_LocalLeaveAuthorizedVlan(lport))
        || (FALSE == NETACCESS_VM_LocalDisableAutoLearningAndTrap2CPU(lport, sm_p->running_port_mode))
       )
    {
        return FALSE;
    }
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalLeaveAuthorizedVlan
 * ---------------------------------------------------------------------
 * PURPOSE  : Remove the port from authorized VLAN, e.g., guest VLAN
 *            auto VLAN, auth-fail VLAN...
 * INPUT    : lport -- lport index
 * OUTPUT   : None.
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalLeaveAuthorizedVlan(UI32_T lport)
{
    UI8_T applied_md5[16] = {0};
    UI8_T null_md5[16] = {0};
    BOOL_T ret;

    if (!NETACCESS_OM_GetDynamicVlanMd5(lport, applied_md5))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("Wrong port(%lu)", lport);
    }

    if (memcmp(null_md5, applied_md5, 16) == 0)
    {
        return TRUE;
    }

    ret = PORTAUTHSRVC_MGR_Vlan_SetToAdmin(lport);

    if (!ret)
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("PORTAUTHSRVC_MGR_Vlan_SetToAdmin(%lu) failed", lport);
    }

    NETACCESS_OM_ClearDynamicVlanMd5(lport);
    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_MGR_LocalCheckManualPortMoveIssue
 *-------------------------------------------------------------------------
 * PURPOSE  : check auto port move issue
 * INPUT    : original_port_mode, new_port_mode
 * OUTPUT   : none
 * RETURN   : TRUE - can learn; FALSE - should not learn
 * NOTE     : reference: port_mode_and_auto_port_move.xls
 *-------------------------------------------------------------------------*/
static BOOL_T NETACCESS_VM_LocalCheckAutoPortMoveIssue(UI32_T original_port_mode, UI32_T new_port_mode, BOOL_T is_from_staic, BOOL_T is_to_static)
{
    if(TRUE == is_from_staic)
    {
        if(TRUE == is_to_static)
        {
            return port_move_ar[original_port_mode-1][new_port_mode-1].static2staic;
        }
        else
        {
            return port_move_ar[original_port_mode-1][new_port_mode-1].static2dynamic;
        }

    }
    else
    {
        if(TRUE == is_to_static)
        {
            return port_move_ar[original_port_mode-1][new_port_mode-1].dynamic2staic;
        }
        else
        {
            return port_move_ar[original_port_mode-1][new_port_mode-1].dynamic2dynamic;
        }
    }

    return FALSE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_LocalIsPortOperUp
 *-------------------------------------------------------------------------
 * PURPOSE  : whether lport is oper-up
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE - yes; FALSE - no
 * NOTE     : None
 *-------------------------------------------------------------------------*/
static BOOL_T NETACCESS_VM_LocalIsPortOperUp(UI32_T lport)
{
    UI32_T      port_status;

    /* check if port oper up
     */
    if ((FALSE == SWCTRL_GetPortOperStatus(lport, &port_status)) ||
        (VAL_ifOperStatus_up != port_status))
    {
        return FALSE;
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_LocalPortDown
 *-------------------------------------------------------------------------
 * PURPOSE  : do something when port link-down or admin-down
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTE     : None
 *-------------------------------------------------------------------------*/
static BOOL_T NETACCESS_VM_LocalPortDown(UI32_T lport)
{
    UI32_T                      unit, port, trunk_id;
    NETACCESS_OM_StateMachine_T state_machine;

#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
    if ((SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) &&
          SWCTRL_LPORT_TRUNK_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id)) ||
#else
    if ((SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id)) ||
#endif
        (FALSE == NETACCESS_OM_GetPortStateMachine(lport, &state_machine)))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("Wrong lport(%lu)", lport);
        return FALSE;
    }

    if (FALSE == NETACCESS_VM_DeleteAllAuthorizedUserByPort(lport))
    {
        return FALSE;
    }

    NETACCESS_OM_ClearStateMachineDot1xLogonFlag(lport);
    NETACCESS_OM_StopStateMachineDoAuthentication(lport);
    NETACCESS_OM_DestroyAllPortEapData(lport);

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETWORKACCESS_VM_LocalGetNextMacFromAmtr
 *-------------------------------------------------------------------------
 * PURPOSE  : get next mac from AMTR
 * INPUT    : mac, vid
 * OUTPUT   : lport, vid
 * RETURN   : TRUE - pass; FALSE - fail
 * NOTE     : None
 *-------------------------------------------------------------------------*/
static BOOL_T NETACCESS_VM_LocalGetNextMacFromAmtr(const UI8_T *mac, UI32_T *lport, UI32_T *vid)
{
    AMTR_TYPE_AddrEntry_T    addr_entry;

    if ((NULL == mac) || (NULL == lport) || (NULL == vid))
        return FALSE;

    memset(&addr_entry, 0, sizeof(AMTR_TYPE_AddrEntry_T));
    memcpy(addr_entry.mac, mac, SYS_ADPT_MAC_ADDR_LEN);
    addr_entry.vid = *vid;

    if (FALSE == AMTR_MGR_GetNextMVAddrEntry(&addr_entry, AMTR_MGR_GET_ALL_ADDRESS))
        return FALSE;

    if (0 != memcmp(addr_entry.mac, mac, SYS_ADPT_MAC_ADDR_LEN))
        return FALSE;

    *lport = addr_entry.ifindex;
    *vid = addr_entry.vid;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalTriggerDot1xSendEapRequest
 * ---------------------------------------------------------------------
 * PURPOSE: trigger dot1x to send EAP-Request/Id packet to client
 * INPUT:  state_machine
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalTriggerDot1xSendEapRequest(
    UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    BOOL_T      ret = FALSE;
#if (SYS_CPNT_DOT1X == TRUE)
    UI32_T      cookie;
    UI16_T      tag_info;
    UI8_T       src_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T       dst_mac[SYS_ADPT_MAC_ADDR_LEN] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x03};
    VLAN_OM_Dot1qPortVlanEntry_T    port_vlan_entry;

    /* check if input is valid
     */
    if (NULL == state_machine)
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("Null Pointer");
        return FALSE;
    }

    memcpy(src_mac,
           (1 == state_machine->port_security_sm.event_bitmap.new_mac) ?
           state_machine->port_security_sm.new_mac_msg->m_newmac_data->src_mac :
           state_machine->port_security_sm.new_mac_msg->m_eap_data->src_mac,
           sizeof(src_mac));

    NETACCESS_VM_DEBUG_PRINT_TRACE("start\n"
                                   "  lport:%lu, mac:%02X-%02X-%02X-%02X-%02X-%02X",
                                   lport,
                                   src_mac[0],
                                   src_mac[1],
                                   src_mac[2],
                                   src_mac[3],
                                   src_mac[4],
                                   src_mac[5]);

    /* get port VLAN
     */
    if (VLAN_POM_GetDot1qPortVlanEntry(lport ,&port_vlan_entry) == FALSE)
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("VLAN_POM_GetDot1qPortVlanEntry(%lu) failed",
                                       lport);
        return FALSE;
    }

    tag_info = port_vlan_entry.dot1q_pvid_index;

    /* get cookie
     */
    if (FALSE == NETACCESS_OM_GetDot1xAuthorizedResultCookie(&cookie))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR(
                        "NETACCESS_OM_GetDot1xAuthorizedResultCookie() failed");
        return FALSE;
    }

    /* tigger dot1x send eap-request
     * if reauth, no msg will exist (only authenticating_mac is available)
     */
    ret = DOT1X_VM_AsyncAuthCheck(lport,      /*state_machine->port_security_sm.authenticating_mac*/
                                  src_mac,
                                  dst_mac,
                                  tag_info,
                                  NETACCESS_VM_EAP_FRAME_TYPE,
                                  NULL,
                                  0,
                                  cookie);

    if (FALSE == ret)
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("DOT1X_VM_AsyncAuthCheck failed\n"
                                       "  lport:%lu, mac:%02X-%02X-%02X-%02X-%02X-%02X",
                                       lport,
                                       src_mac[0],
                                       src_mac[1],
                                       src_mac[2],
                                       src_mac[3],
                                       src_mac[4],
                                       src_mac[5]);
    }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    return ret;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalForwardEapPacketToDot1x
 * ---------------------------------------------------------------------
 * PURPOSE: forward EAP-Packet to dot1x
 * INPUT:  state_machine
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalForwardEapPacketToDot1x(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
#if (SYS_CPNT_DOT1X == TRUE)
    UI32_T                  debug_flag = 0;
    UI32_T                  cookie;
    NETACCESS_EAP_DATA_T    *eap_data;
    BOOL_T                  ret;

    /* check if input is valid
     */
    if (NULL == state_machine)
    {
        return FALSE;
    }

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* get cookie
     */
    if (FALSE == NETACCESS_OM_GetDot1xAuthorizedResultCookie(&cookie))
    {
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] NETACCESS_OM_GetDot1xAuthorizedResultCookie() failed", __FUNCTION__);
        return FALSE;
    }

    /* check if there is eap data
     */
    if ((NULL == state_machine->port_security_sm.new_mac_msg) ||
        (NULL == state_machine->port_security_sm.new_mac_msg->m_eap_data))
    {
        /* if has no m_eap_data, try to find one from om
         */
        eap_data = NETACCESS_OM_GetFirstPortEapData(lport);
        if (NULL == eap_data)
        {
            if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
                printf("\r\n[%s] eap data not found(%lu)", __FUNCTION__, lport);
            return FALSE;
        }

        ret = FALSE;

        /* forward all eap packets to dot1x
         */
        for ( ; NULL != eap_data; eap_data = NETACCESS_OM_GetFirstPortEapData(lport))
        {
            /* delete om data
             */
            NETACCESS_OM_DestroyFirstPortEapPacket(lport);

            ret = DOT1X_VM_AsyncAuthCheck(eap_data->lport_no, eap_data->src_mac, eap_data->dst_mac,
                        eap_data->tag_info, eap_data->type, eap_data->pkt_data, eap_data->pkt_length, cookie);

            if (FALSE == ret)
            {
                if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
                {
                    printf("\r\n[%s] DOT1X_VM_AsyncAuthCheck failed\r\n\tport:%lu mac:", __FUNCTION__, eap_data->lport_no);
                    NETACCESS_VM_LocalPrintMacAddr(eap_data->src_mac);
                }

                /* free memory
                 */
                NETACCESS_OM_FreeEapData(&eap_data);
                return FALSE;
            }

            /* free memory
             */
            NETACCESS_OM_FreeEapData(&eap_data);
        }
    }
    else
    {
        eap_data = state_machine->port_security_sm.new_mac_msg->m_eap_data;

        /* forward eap packet to dot1x
         */
        ret = DOT1X_VM_AsyncAuthCheck(eap_data->lport_no, eap_data->src_mac, eap_data->dst_mac,
                    eap_data->tag_info, eap_data->type, eap_data->pkt_data, eap_data->pkt_length, cookie);

        /* all these EAP packets in queue should be obsoleted
         * because a latest EAP existed implies
         * 802.1x client state machine had changed its own state (e.g. retry-timeout)
         */
        NETACCESS_OM_DestroyAllPortEapData(eap_data->lport_no);

        if (FALSE == ret)
        {
            if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            {
                printf("\r\n[%s] DOT1X_VM_AsyncAuthCheck failed\r\n\tport:%lu mac:", __FUNCTION__, eap_data->lport_no);
                NETACCESS_VM_LocalPrintMacAddr(eap_data->src_mac);
            }
        }
    }

    return ret;
#else
    return TRUE;
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */
}

#if 0
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalNotifyDot1xSendEapResult
 * ---------------------------------------------------------------------
 * PURPOSE: tigger dot1x to send EAP-Success or EAP-Failure
 * INPUT:  state_machine, is_succeeded
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalNotifyDot1xSendEapResult(UI32_T lport, UI8_T *mac, BOOL_T is_succeeded)
{
    DOT1X_SM_AUTH_Obj_T *sm_p;
    UI32_T      debug_flag = 0;

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* check if input is valid
     */
    if (NULL == mac)
    {
        return FALSE;
    }

    sm_p = DOT1X_OM_GetSMObjByPortMac(lport, mac);
    if (NULL == sm_p)
    {
        return FALSE;
    }

    NETACCESS_DBG1(NETACCESS_OM_DEBUG_VM_IFO, "Notify to send EAPOL FAILURE on lport(%lu)", lport);

    DOT1X_SM_AUTH_Go(sm_p, DOT1X_SM_AUTH_EAPLOGOFF_EV);

    return TRUE;
}
#endif /* #if 0 */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalTriggerRadaDoAuthentication
 * ---------------------------------------------------------------------
 * PURPOSE: trigger rada do authentication
 * INPUT:  state_machine
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalTriggerRadaDoAuthentication(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    UI32_T      debug_flag = 0, rada_auth_mode, cookie;
    UI8_T       *mac;
    char        *user_name, *password;
    const UI8_T mac_string_len = 17;
    BOOL_T      ret;

    /* check if input is valid
     */
    if (NULL == state_machine)
    {
        return FALSE;
    }

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* get cookie
     */
    if (FALSE == NETACCESS_OM_GetRadaAuthorizedResultCookie(&cookie))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR(
                         "NETACCESS_OM_GetRadaAuthorizedResultCookie() failed");
        return FALSE;
    }

    /* get auth mode
     */
    if (FALSE == NETACCESS_OM_GetSecureAuthMode(&rada_auth_mode))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR(
                                 "NETACCESS_OM_GetSecureRadaAuthMode() failed");
        return FALSE;
    }

    /* allocate memory
     */
    user_name = L_MM_Malloc(
        ((NETACCESS_VM_MAX_LEN_OF_AUTH_USERNAME > mac_string_len) ? NETACCESS_VM_MAX_LEN_OF_AUTH_USERNAME : mac_string_len) + 1,
        L_MM_USER_ID2(SYS_MODULE_NETACCESS, NETACCESS_TYPE_TRACE_ID_NETACCESS_VM_LOCALTRIGGERRADADOAUTHENTICATION));

    if (NULL == user_name)
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("L_MM_Malloc() failed");
        return FALSE;
    }

    password = L_MM_Malloc(
        ((NETACCESS_VM_MAX_LEN_OF_AUTH_PASSWORD > mac_string_len) ? NETACCESS_VM_MAX_LEN_OF_AUTH_PASSWORD : mac_string_len) + 1,
        L_MM_USER_ID2(SYS_MODULE_NETACCESS, NETACCESS_TYPE_TRACE_ID_NETACCESS_VM_LOCALTRIGGERRADADOAUTHENTICATION));

    if (NULL == password)
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("L_MM_Malloc() failed");

        L_MM_Free(user_name);
        return FALSE;
    }

    mac = state_machine->port_security_sm.authenticating_mac;

    /* fill username and password
     */
    switch (rada_auth_mode)
    {
        case NETACCESS_AUTHMODE_MACADDRESS:
            sprintf(user_name, "%02X-%02X-%02X-%02X-%02X-%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            sprintf(password,  "%02X-%02X-%02X-%02X-%02X-%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            break;

        case NETACCESS_AUTHMODE_FIXED:
            break;

        default:
            NETACCESS_VM_DEBUG_PRINT_ERROR("Unknown rada_auth_mode(%lu)",
                                           rada_auth_mode);
            return FALSE;
    }

    NETACCESS_VM_DEBUG_PRINT_TRACE("Call RADIUS CSC to do authentication");

    /* tigger rada do authentication
     * if reauth, no msg will exist (only authenticating_mac is available)
     */
    ret = RADIUS_PMGR_RadaAuthCheck(lport, mac, user_name, password, cookie);

    NETACCESS_VM_DEBUG_PRINT_TRACE("RADIU return %d", ret);

    L_MM_Free(user_name);
    L_MM_Free(password);

    if (FALSE == ret)
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("RADIUS_PMGR_RadaAuthCheck failed\r\n"
                                       "lport(%lu), MAC(%02X-%02X-%02X-%02X-%02X-%02X)",
                                       lport,
                                       mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }

    return ret;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalLearnNewMac
 * ---------------------------------------------------------------------
 * PURPOSE  : Set the authenticating MAC address to AMTR and OM (hisam).
 * INPUT    : lport         -- logic port number
 *            state_machine -- state machine pointer
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : If the authenticating MAC address is in hisam,
 *            this function should update expire time fields of the MAC address
 *            entry by state_machine.
 *            If the authenticating MAC address isn't in hisam,
 *            this function should create new entry for OM and AMTR.
 *
 *            If exceeded the max acceptable MAC number, returns FALSE.
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalLearnNewMac(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    NETACCESS_OM_SecureKey_T        key;

    memset(&key, 0, sizeof(key));
    key.lport  = lport;
    memcpy(key.secure_mac, state_machine->port_security_sm.authenticating_mac, SYS_ADPT_MAC_ADDR_LEN);

    /* check if MAC address exist in HISAM table
     */
    if (TRUE == NETACCESS_OM_DoesRecordExistInHisamBySecureKey(&key))
    {
        if (FALSE == NETACCESS_VM_LocalAuthorizeExistedMac(lport, state_machine))
        {
            return FALSE;
        }
    }
    else
    {
        NETACCESS_OM_SecureMacEntry_T mac_entry;
        memset(&mac_entry, 0, sizeof(mac_entry));

        if (   (FALSE == NETACCESS_VM_LocalCollectMacEntry(lport, state_machine, &mac_entry))
            || (TRUE == NETACCESS_VM_LocalIsSecureAddressTableFull(lport, &mac_entry))
           )
        {
            return FALSE;
        }

        /* Write to OM and hisam
         */
        if (FALSE == NETACCESS_OM_CreateSecureAddressEntry(&mac_entry))
        {
            NETACCESS_VM_DEBUG_PRINT_ERROR("NETACCESS_OM_CreateSecureAddressEntry failed");
            return FALSE;
        }
    }

    if (!NETACCESS_VM_ApplyVlanQos(lport, state_machine))
    {
        NETACCESS_VM_LocalDeleteAuthenticatingMac(lport, state_machine);
        return FALSE;
    }

    /* Write to AMTR
     */
    if (FALSE == NETACCESS_VM_LocalAddAuthenticatingMacToAmtr(lport, state_machine))
    {
        return FALSE;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalIsSecureAddressTableFull
 * ---------------------------------------------------------------------
 * PURPOSE: Check whether secure address is full or not
 * INPUT  : lport       -- logic port number
 *          mac_entry_p -- New MAC address entry pointer
 * OUTPUT : None
 * RETURN : If the secure address table is full, the return value is TRUE.
 *          If the secure address table isn't full yet, the return value is FALSE.
 * NOTES  : This function should return TRUE, if there have an error happened.
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalIsSecureAddressTableFull(UI32_T lport, NETACCESS_OM_SecureMacEntry_T* mac_entry_p)
{
    UI32_T  running_port_mode;
    BOOL_T  is_full = FALSE;

    if (FALSE == NETACCESS_OM_GetPortStateMachineRunningPortMode(lport, &running_port_mode))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("NETACCESS_OM_GetPortStateMachineRunningPortMode failed");
        return TRUE;
    }

    /* check if MAC table is full by port mode
     */
    switch (running_port_mode)
    {
        case NETACCESS_PORTMODE_NO_RESTRICTIONS:
        case NETACCESS_PORTMODE_CONTINUOS_LEARNING:
        case NETACCESS_PORTMODE_AUTO_LEARN:

            /* No check
             */
            break;

        case NETACCESS_PORTMODE_SECURE:

            /* Can't learn MAC in secure mode
             */
            NETACCESS_VM_DEBUG_PRINT_ERROR("Can't learn mac in secure mode");
            is_full = TRUE;
            break;

        case NETACCESS_PORTMODE_USER_LOGIN:
            break;

        case NETACCESS_PORTMODE_USER_LOGIN_SECURE:
        case NETACCESS_PORTMODE_USER_LOGIN_WITH_OUI:
        case NETACCESS_PORTMODE_MAC_ADDRESS_WITH_RADIUS:
        case NETACCESS_PORTMODE_MAC_ADDRESS_OR_USER_LOGIN_SECURE:
        case NETACCESS_PORTMODE_MAC_ADDRESS_ELSE_USER_LOGIN_SECURE:
        case NETACCESS_PORTMODE_PORT_SECURITY:

            /* When the address limit is reached, new MAC address SHALL be treated
             * as authentication fail.
             */
            if(TRUE == NETACCESS_OM_IsSecureAddressesFull(lport))
            {
                NETACCESS_VM_DEBUG_PRINT_ERROR("Secure address table full");
                is_full = TRUE;
            }

            /* MACAuth and dot1x are enabled on the same port
             */
            if (    (FALSE == is_full)
                &&  (NETACCESS_PORTMODE_MAC_ADDRESS_OR_USER_LOGIN_SECURE == running_port_mode)
               )
            {
                is_full = (1 == mac_entry_p->mac_flag.auth_by_rada) ?
                          NETACCESS_VM_LocalIsMacAuthMaxMacCountReached(lport) :
                          NETACCESS_VM_LocalIsDot1xMaxMacCountReached(lport);
            }

            break;

        case NETACCESS_PORTMODE_MAC_AUTHENTICATION:

            if(TRUE == NETACCESS_OM_IsSecureAddressesFull(lport))
            {
                NETACCESS_VM_DEBUG_PRINT_ERROR("Secure address table full");
                is_full = TRUE;
            }

            if (FALSE == is_full)
            {
                is_full = NETACCESS_VM_LocalIsMacAuthMaxMacCountReached(lport);
            }

            break;

        case NETACCESS_PORTMODE_DOT1X:

            if(TRUE == NETACCESS_OM_IsSecureAddressesFull(lport))
            {
                NETACCESS_VM_DEBUG_PRINT_ERROR("Secure address table full");
                is_full = TRUE;
            }

            if (FALSE == is_full)
            {
                is_full = NETACCESS_VM_LocalIsDot1xMaxMacCountReached(lport);
            }

            break;

        default:
            NETACCESS_VM_DEBUG_PRINT_ERROR("Wrong port mode(%lu)", running_port_mode);
            return TRUE;
    }

    return is_full;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalIsMacAuthMaxMacCountReached
 * ---------------------------------------------------------------------
 * PURPOSE: Check whether MACAuth max MAC count is reached or not
 * INPUT  : lport       -- logic port number
 * OUTPUT : None
 * RETURN : If the MACAuth max MAC count is reached, the return value is TRUE.
 *          If the MACAuth max MAC count isn't reached yet, the return value is FALSE.
 * NOTES  : This function should return TRUE, if there have an error happened.
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalIsMacAuthMaxMacCountReached(UI32_T lport)
{
    UI32_T max_count;
    BOOL_T is_full = FALSE;

    if (FALSE == NETACCESS_OM_GetMacAuthPortMaxMacCount(lport, &max_count))
    {
        return TRUE;
    }

    if (max_count <= NETACCESS_VM_GetSecureNumberAddressesStoredAuthorByRada(lport))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("MAX MAC count(%lu) of MACAuth is reached",
            max_count);
        is_full = TRUE;
    }

    return is_full;
}

#if (SYS_CPNT_DOT1X == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_VM_LocalIsDot1xMultiHostMaxMacCountReached
 * ---------------------------------------------------------------------
 * PURPOSE: Check whether dot1x multi-host max MAC count is reached or not
 * INPUT  : lport -- logic port number
 * OUTPUT : None
 * RETURN : TRUE  -- reached
 *          FALSE -- not yet
 * NOTES  : This function should return TRUE, if there have an error happened.
 *          The count is equal to max MAC count + 1(dot1x supplicant)
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalIsDot1xMultiHostMaxMacCountReached(UI32_T lport)
{
    UI32_T total_stored_count;
    UI32_T dot1x_stored_count;
    UI32_T dot1x_max_count;

    if (   (FALSE == DOT1X_OM_Get_PortMultiHostMacCount(lport, &dot1x_max_count))
        || (FALSE == NETACCESS_OM_GetSecureNumberAddressesStored(lport, &total_stored_count)))
    {
        return TRUE;
    }

    dot1x_stored_count = total_stored_count - NETACCESS_VM_GetSecureNumberAddressesStoredAuthorByRada(lport);

    if (dot1x_stored_count >= (dot1x_max_count + 1))
    {
        return TRUE;
    }

    return FALSE;
}
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

/* ---------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_VM_LocalIsDot1xMaxMacCountReached
 * ---------------------------------------------------------------------
 * PURPOSE: Check whether dot1x max MAC count is reached or not
 * INPUT  : lport -- logic port number
 * OUTPUT : None
 * RETURN : TRUE  -- reached
 *          FALSE -- not yet
 * NOTES  : This function should return TRUE, if there have an error happened.
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalIsDot1xMaxMacCountReached(UI32_T lport)
{
#if (SYS_CPNT_DOT1X == TRUE)
    UI32_T operation_mode;

    if (FALSE == DOT1X_OM_Get_PortOperationMode(lport, &operation_mode))
    {
        return TRUE;
    }

    if (   (DOT1X_PORT_OPERATION_MODE_MULTIPASS == operation_mode)
        && (TRUE == NETACCESS_VM_LocalIsDot1xMultiHostMaxMacCountReached(lport)))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("MAX MAC Count of dot1x is reached");
        return TRUE;
    }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    return FALSE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalLearnPreauthenticatedMac
 * ---------------------------------------------------------------------
 * PURPOSE: create preauthenticated mac (om & hisam)
 * INPUT:  lport, state_machine
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  AMTR manipulation (add addr) must be after VLAN is applied
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalLearnPreauthenticatedMac(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    UI32_T      unit, port, trunk_id;
    NETACCESS_OM_SecureKey_T    key;
    NETACCESS_OM_HISAMentry_T hisam_entry;
    UI32_T debug_flag = 0;

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* check if input is valid
     */
    if ((NULL == state_machine) ||
        (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id)))
    {
        return FALSE;
    }

    if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
        printf("\r\n[%s] start processing...", __FUNCTION__);

    /* whether the (lport + new mac) existed in address table
     */
    key.lport  = lport;
    memcpy(key.secure_mac, state_machine->port_security_sm.authenticating_mac, SYS_ADPT_MAC_ADDR_LEN);

    /* check if MAC address exist in HISAM table
     */
    if (TRUE == NETACCESS_OM_GetHisamRecordBySecureKey(&key, &hisam_entry))
    {
        if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
            printf("\r\n[%s] secure mac entry exist.", __FUNCTION__);

        /* write amtr again and return
         */
        NETACCESS_VM_LocalAddAuthenticatingMacToAmtr(lport, state_machine);
        return TRUE;
    }

    /* check if MAC table is full
     */
    if (FALSE == NETACCESS_OM_IsSecureAddressesFull(lport))
    {
        if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
            printf("\r\n[%s] create new preauth mac entry...", __FUNCTION__);

        /* create a new mac entry
         */
        return NETACCESS_VM_LocalCreateNewPreauthMac(lport, state_machine);
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalCreateNewPreauthMac
 * ---------------------------------------------------------------------
 * PURPOSE: Create the authenticating MAC address to AMTR and OM with
            MAC filter attribute.
 * INPUT  : lport           -- logic port number
 *          state_machine_p -- state machine pointer
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalCreateNewPreauthMac(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine_p)
{
    UI32_T unit, port, trunk_id;
    NETACCESS_OM_SecureMacEntry_T  mac_entry;
    UI32_T debug_flag = 0;

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* check if input is valid
     */
    if ((NULL == state_machine_p) ||
        (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id)))
    {
        return FALSE;
    }

    memset(&mac_entry, 0, sizeof(NETACCESS_OM_SecureMacEntry_T));
    mac_entry.lport = lport;
    memcpy(mac_entry.secure_mac, state_machine_p->port_security_sm.authenticating_mac, SYS_ADPT_MAC_ADDR_LEN);
    mac_entry.addr_row_status = NETACCESS_ROWSTATUS_ACTIVE;
    mac_entry.authorized_status = NETACCESS_ROWSTATUS_ACTIVE;
    SYS_TIME_GetRealTimeBySec(&mac_entry.record_time);
    mac_entry.session_time        = NETACCESS_MAX_SESSION_TIME;
    mac_entry.session_expire_time = NETACCESS_MAX_SESSION_EXPIRE_TIME;
    mac_entry.mac_flag.is_mac_filter_mac = 1;

    if(FALSE == NETACCESS_OM_CreateSecureAddressEntry(&mac_entry))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("NETACCESS_OM_CreateSecureAddressEntry failed");
        return FALSE;
    }

    /* add to AMTR
     */
    if (FALSE == NETACCESS_VM_LocalAddAuthenticatingMacToAmtr(lport, state_machine_p))
    {
        return FALSE;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalAddAuthenticatingMacToAmtr
 * ---------------------------------------------------------------------
 * PURPOSE: Write the authenticating MAC address to AMTR.
 * INPUT  : lport         -- logic port number
 *          state_machine -- state machine pointer
 * OUTPUT: None.
 * RETURN : If the function succeeds, the return value is TRUE. If the
 *          specified VLAN doesn't join lport, returns TRUE, even the
 *          MAC address doesn't be written.
 *          If the function fails, the return value is FALSE.
 * NOTES  : This authenticating MAC address should exist in secure address
            table first.
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalAddAuthenticatingMacToAmtr(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    UI32_T                          vlan_ifindex;
    AMTR_TYPE_AddressLifeTime_T     address_life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET;
    NETACCESS_VM_AuthResult_T auth_result;
    NETACCESS_OM_SecureMacEntry_T   mac_entry;
    BOOL_T                          ret = FALSE;

    /* Check if input is valid
     */
    if (NULL == state_machine)
    {
        return FALSE;
    }

    auth_result.result = SYS_CALLBACK_MGR_AUTH_FAILED;
    auth_result.vid = 0;

    NETACCESS_VM_DEBUG_PRINT_TRACE(
                         "lport(%lu) add a MAC (%02x-%02x-%02x-%02x-%02x-%02x)",
                         lport,
                         state_machine->port_security_sm.authenticating_mac[0],
                         state_machine->port_security_sm.authenticating_mac[1],
                         state_machine->port_security_sm.authenticating_mac[2],
                         state_machine->port_security_sm.authenticating_mac[3],
                         state_machine->port_security_sm.authenticating_mac[4],
                         state_machine->port_security_sm.authenticating_mac[5]);

    /* Check if port mode can add secure mac address
     */
    if(0 == netaccess_vm_port_mode_ar[state_machine->running_port_mode-1].is_allow_add_secure_address)
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR(
                           "Port mode(%lu) should not enable software learning",
                           state_machine->running_port_mode);
        auth_result.result = SYS_CALLBACK_MGR_AUTH_UNAUTHENTICATED;
        ret = FALSE;
        goto WRITE_FAILED;
    }

    mac_entry.lport = lport;
    memcpy(mac_entry.secure_mac, state_machine->port_security_sm.authenticating_mac, SYS_ADPT_MAC_ADDR_LEN);

    /* Get MAC address entry
     */
    if (FALSE == NETACCESS_OM_GetSecureAddressEntry(&mac_entry))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR(
                               "Authenticating MAC address is not exist in OM");
        auth_result.result = SYS_CALLBACK_MGR_AUTH_UNAUTHENTICATED;
        ret = FALSE;
        goto WRITE_FAILED;
    }

    /* Convert the VLAN ID
     * If the flag is_tagged is 1, the VID is the source VLAND ID.
     * If the flag is_tagged is 0, the VID is the current PVID.
     */
    if(1 == state_machine->port_security_sm.event_bitmap.is_tagged)
    {
        /* tag packet,use src_vid in packet
         */
        auth_result.vid = state_machine->port_security_sm.src_vid;
    }
    else
    {
        VLAN_OM_Vlan_Port_Info_T         port_info;

        /* untag packet,use current pvid
         */
        memset(&port_info, 0, sizeof(VLAN_OM_Vlan_Port_Info_T));
        if(FALSE == VLAN_PMGR_GetPortEntry(lport, &port_info))
        {
            NETACCESS_VM_DEBUG_PRINT_ERROR("VLAN_PMGR_GetPortEntry failed");
            auth_result.result = SYS_CALLBACK_MGR_AUTH_UNAUTHENTICATED;
            ret = FALSE;
            goto WRITE_FAILED;
        }

        auth_result.vid = port_info.port_item.dot1q_pvid_index;
    }

    /* AMTR design changed.
     * never assume AMTR behavior here,
     * see AMTR_MGR_VLAN_VALIDATION for more info.
     */

    /* If user's incoming VLAN doesn't join to the port after
     * be applied the returned VLAN configuration, then the
     * user's MAC address shall not be written to AMTR.
     */
    VLAN_VID_CONVERTTO_IFINDEX(auth_result.vid, vlan_ifindex);
    if( FALSE == VLAN_OM_IsPortVlanMember(vlan_ifindex, lport))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR( "Port(%lu) is not VLAN(%lu) member", lport, auth_result.vid);

        auth_result.result = SYS_CALLBACK_MGR_AUTH_UNAUTHENTICATED;
        auth_result.vid = 0;
        ret = TRUE;
        goto WRITE_FAILED;
    }


    /* Check AMTR before add the authenticating MAC address
     */
    {
        AMTR_TYPE_AddrEntry_T   addr_entry;

        memset(&addr_entry, 0, sizeof(addr_entry));
        memcpy(addr_entry.mac, state_machine->port_security_sm.authenticating_mac, SYS_ADPT_MAC_ADDR_LEN);
        addr_entry.ifindex = lport;
        addr_entry.vid = auth_result.vid;

        /* check if the authenticated MAC is existed on AMTR
         */
        if (TRUE == AMTR_MGR_GetExactIfIndexAddrEntry(&addr_entry))
        {
            if(1 == mac_entry.mac_flag.authorized_by_dot1x)
            {
                address_life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET;
            }
            else
            {
                address_life_time = NETACCESS_VM_SecureAddrLifeTime( NETACCESS_VM_LocalAgingModeOfSecureAddress() );
            }
            auth_result.result = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT == address_life_time ?
                                                        SYS_CALLBACK_MGR_AUTH_AUTHENTICATED_TEMPORARILY :
                                                        SYS_CALLBACK_MGR_AUTH_AUTHENTICATED;

            ret = TRUE;
            goto AUTH_MAC_EXISTED;
        }
    }

    /* add MAC address to AMTR
     */
   if (     (NETACCESS_PORTMODE_PORT_SECURITY == state_machine->running_port_mode)
        || (1 == mac_entry.mac_flag.authorized_by_dot1x)
      )
    {
        /* attribute is DELETE_ON_RESET, no age out
         */
        address_life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET;
        auth_result.result       = SYS_CALLBACK_MGR_AUTH_AUTHENTICATED;
    }
    else
    {
        /*  attribute is DELETE_ON_TIMEOUT,
         *  use PSEC's setting to set the age out time
         */
        address_life_time = NETACCESS_VM_SecureAddrLifeTime( NETACCESS_VM_LocalAgingModeOfSecureAddress() );
        auth_result.result = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT == address_life_time ?
                                                            SYS_CALLBACK_MGR_AUTH_AUTHENTICATED_TEMPORARILY :
                                                            SYS_CALLBACK_MGR_AUTH_AUTHENTICATED;
    }

    if (NULL != state_machine->port_security_sm.cookie)
    {
        ret = TRUE;
        goto AUTH_SUCCEED;
    }
    else
    {
        /*  attribute is DELETE_ON_TIMEOUT,
         *      use PSEC's setting to set the age out time
         */
        AMTR_TYPE_AddrEntry_T   addr_entry;

        addr_entry.life_time=   address_life_time/* NETACCESS_VM_SecureAddrLifeTime( NETACCESS_VM_LocalAgingModeOfSecureAddress() )*/;
        memcpy(addr_entry.mac, state_machine->port_security_sm.authenticating_mac, AMTR_TYPE_MAC_LEN);
        addr_entry.ifindex  =   lport;
        addr_entry.vid      =   auth_result.vid;
        addr_entry.source   =   NETACCESS_VM_SecureAddrSourceType();
        addr_entry.action   =   AMTR_TYPE_ADDRESS_ACTION_FORWARDING;

        ret = AMTR_MGR_SetAddrEntry (&addr_entry);
    }


    NETACCESS_VM_DEBUG_PRINT_IMPORTANT("Write MAC(%02X-%02X-%02X-%02X-%02X-%02X),"
                                       " vid(%lu), lport(%lu) %s",
                                       state_machine->port_security_sm.authenticating_mac[0],
                                       state_machine->port_security_sm.authenticating_mac[1],
                                       state_machine->port_security_sm.authenticating_mac[2],
                                       state_machine->port_security_sm.authenticating_mac[3],
                                       state_machine->port_security_sm.authenticating_mac[4],
                                       state_machine->port_security_sm.authenticating_mac[5],
                                       auth_result.vid, lport,
                                       (ret==TRUE)?"succeeded":"faild");

    if(FALSE == ret)
    {
        /* don't increase vid counter and remove secure MAC address if vid counter is zero
         */
        if(0 == mac_entry.vlan_counter)
        {
            /* remove mac entry
             */
            NETACCESS_OM_DeleteSecureAddressEntryByIndex(mac_entry.mac_index);
        }
        auth_result.result = SYS_CALLBACK_MGR_AUTH_UNAUTHENTICATED;
        ret = FALSE;
        goto WRITE_FAILED;
    }

AUTH_SUCCEED:
    /* update vid counter
     */
    if (FALSE == NETACCESS_OM_IncSecureAddrVidCounterByIndex(mac_entry.mac_index))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("NETACCESS_OM_IncSecureAddrVidCounterByIndex failed");
        auth_result.result = SYS_CALLBACK_MGR_AUTH_UNAUTHENTICATED;
        ret = FALSE;
        goto WRITE_FAILED;
    }

    /* update om (write_to_amtr)
     */
    if (FALSE == NETACCESS_OM_SetSecureAddrWriteToAmtrByIndex(mac_entry.mac_index, TRUE))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("NETACCESS_OM_SetSecureAddrWriteToAmtrByIndex failed");
        auth_result.result = SYS_CALLBACK_MGR_AUTH_UNAUTHENTICATED;
        ret = FALSE;
        goto WRITE_FAILED;
    }

AUTH_MAC_EXISTED:
WRITE_FAILED:

    if (NULL != state_machine->port_security_sm.cookie)
    {
        NETACCESS_VM_LocalPassCookieToNextCSC(lport,
                                              state_machine,
                                              &auth_result);
    }

    return ret;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalDeleteAuthenticatingMac
 * ---------------------------------------------------------------------
 * PURPOSE: Delete the authenticating MAC address from AMTR and OM.
 * INPUT  : lport           -- logic port number
 *          state_machine   -- state machine pointer
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalDeleteAuthenticatingMac(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine_p)
{
    NETACCESS_OM_SecureKey_T           key;

    key.lport  = lport;
    memcpy(key.secure_mac, state_machine_p->port_security_sm.authenticating_mac, SYS_ADPT_MAC_ADDR_LEN);
    if (TRUE == NETACCESS_OM_DoesRecordExistInHisamBySecureKey(&key))
    {
        NETACCESS_OM_SecureMacEntry_T mac_entry;

        memset(&mac_entry, 0, sizeof(mac_entry));
        mac_entry.lport = key.lport;
        memcpy(mac_entry.secure_mac, key.secure_mac, SYS_ADPT_MAC_ADDR_LEN);
        NETACCESS_OM_GetSecureAddressEntry(&mac_entry);

        if (FALSE == NETACCESS_VM_DeleteAuthorizedUserByIndex(mac_entry.mac_index))
        {
            return FALSE;
        }
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalExecIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE: execute intrusion action (depend on secureIntrusionAction)
 * INPUT:  state_machine
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalExecIntrusionAction(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    UI32_T  debug_flag = 0, intrusion_action;
    UI32_T  sys_time;

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* chcek if input is valid
     */
    if ((NULL == state_machine) ||
        (NETACCESS_STATE_INTRUSION_HANDLING != state_machine->port_security_sm.running_state))
    {
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] null pointer or bad running state", __FUNCTION__);

        return FALSE;
    }

    /* check if port mode can execute intrusion action
     */
    if(0 == netaccess_vm_port_mode_ar[state_machine->running_port_mode-1].is_allow_exec_intrusion_action)
    {
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] port mode cannot execute intrusion action", __FUNCTION__);

        return FALSE;
    }

    /* get intrusion action
     */
    if (FALSE == NETACCESS_OM_GetSecureIntrusionAction(lport, &intrusion_action))
    {
        return FALSE;
    }

    /* take disable port action
     */
    switch (intrusion_action)
    {
        case NETACCESS_INTRUSIONACTION_DISABLE_PORT:
        case NETACCESS_INTRUSIONACTION_TRAP_AND_DISABLE_PORT:
            if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
                printf("\r\n[%s] disablePort(%lu)", __FUNCTION__, lport);

            /* disable port
             */
            if (FALSE == SYS_CALLBACK_MGR_SetPortStatusCallback(SYS_MODULE_NETACCESS, lport, FALSE, SWCTRL_PORT_STATUS_SET_BY_NETACCESS_LINK_DETECTION))
            {
                return FALSE;
            }
            break;

        case NETACCESS_INTRUSIONACTION_DISABLE_PORT_TEMPORARILY:
        case NETACCESS_INTRUSIONACTION_TRAP_AND_DISABLE_PORT_TEMPORARILY:
            if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
                printf("\r\n[%s] disablePortTemporarily(%lu)", __FUNCTION__, lport);

            /* disable port
             */
            if (FALSE == SYS_CALLBACK_MGR_SetPortStatusCallback(SYS_MODULE_NETACCESS, lport, FALSE, SWCTRL_PORT_STATUS_SET_BY_NETACCESS_LINK_DETECTION))
            {
                if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
                    printf("\r\n[%s] SYS_CALLBACK_MGR_SetPortStatusCallback(%lu, down) failed", __FUNCTION__, lport);
                return FALSE;
            }

            /* setup disable port timer
             */
            SYS_TIME_GetRealTimeBySec(&sys_time);
            if (FALSE == NETACCESS_OM_CreatePortDisableTimer(lport, sys_time + NETACCESS_VM_DISABLE_PORT_SECONDS))
            {
                return FALSE;
            }
            break;
        default:
            break;
    }

    /* take trap action
     */
    switch (intrusion_action)
    {
        case NETACCESS_INTRUSIONACTION_TRAP:
        case NETACCESS_INTRUSIONACTION_TRAP_AND_DISABLE_PORT:
        case NETACCESS_INTRUSIONACTION_TRAP_AND_DISABLE_PORT_TEMPORARILY:
            if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
                printf("\r\n[%s] trap(%lu), send trap", __FUNCTION__, lport);

            /* Trap for intrusion mac address
             */
            NETACCESS_VM_LocalSendViolationTrap(lport,state_machine->port_security_sm.authenticating_mac);

            break;
        default:
            break;
    }

    /* take other action
     */
    switch (intrusion_action)
    {
        case NETACCESS_INTRUSIONACTION_NO_ACTION:
        case NETACCESS_INTRUSIONACTION_DISABLE_PORT:
        case NETACCESS_INTRUSIONACTION_TRAP:
        case NETACCESS_INTRUSIONACTION_TRAP_AND_DISABLE_PORT:
        case NETACCESS_INTRUSIONACTION_DISABLE_PORT_TEMPORARILY:
        case NETACCESS_INTRUSIONACTION_TRAP_AND_DISABLE_PORT_TEMPORARILY:
            break;

        case NETACCESS_INTRUSIONACTION_NOT_AVAILABLE:
        default:
            if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
                printf("\r\n[%s] notAvailable(%lu)", __FUNCTION__, lport);

            return FALSE;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalExecMacAuthenticationIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE : execute intrusion action (depend on mac-authentication intrusion action)
 * INPUT   : lport, state_machine
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTES   : none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalExecMacAuthenticationIntrusionAction(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    UI32_T  debug_flag = 0, intrusion_action;

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* chcek if input is valid
     */
    if (NULL == state_machine)
    {
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] null pointer or bad running state", __FUNCTION__);

        return FALSE;
    }

    /* get intrusion action
     */
    if (FALSE == NETACCESS_OM_GetMacAuthPortIntrusionAction(lport, &intrusion_action))
    {
        return FALSE;
    }

    /* take disable port action
     */
    switch (intrusion_action)
    {
        case VAL_macAuthPortIntrusionAction_block_traffic:
            if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
            {
                printf("\r\n[%s] block mac ", __FUNCTION__);
                NETACCESS_VM_LocalPrintMacAddr(state_machine->port_security_sm.authenticating_mac);
            }

            /* delete mac address from OM and AMTR
             */
            if (1 == state_machine->port_security_sm.event_bitmap.waiting_reauth_result) /* reauthentication failed */
            {
                NETACCESS_OM_SecureMacEntry_T entry;

                entry.lport = lport;
                memcpy(entry.secure_mac, state_machine->port_security_sm.authenticating_mac, SYS_ADPT_MAC_ADDR_LEN);

                /* get MAC address entry for delete
                 */
                if(FALSE == NETACCESS_OM_GetSecureAddressEntry(&entry))
                {
                    return FALSE;
                }

                /* delete mac from amtr & om
                 */
                if (FALSE == NETACCESS_VM_DeleteAuthorizedUserByIndex(entry.mac_index))
                {
                    return FALSE;
                }
            }

            /* write to unauthorized cache
             */
            if (FALSE == NETACCESS_OM_AddOneMac2UnauthorizedMacCache(state_machine->port_security_sm.authenticating_mac))
            {
                NETACCESS_VM_DEBUG_PRINT_ERROR("NETACCESS_OM_AddOneMac2UnauthorizedMacCache failed");
                return FALSE;
            }

            break;

        case VAL_macAuthPortIntrusionAction_pass_traffic:
            if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
            {
                printf("\r\n[%s] pass mac ", __FUNCTION__);
                NETACCESS_VM_LocalPrintMacAddr(state_machine->port_security_sm.authenticating_mac);
            }

            if (FALSE == NETACCESS_VM_LocalLearnNewMac(lport, state_machine))
            {
                    return FALSE;
            }
            break;

        default:
            if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
                printf("\r\n[%s] notAvailable(%lu)", __FUNCTION__, lport);
    }

    return TRUE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalAuthorizeExistedMac
 * ---------------------------------------------------------------------
 * PURPOSE: let an existed mac to be authorized
 * INPUT:  state_machine
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalAuthorizeExistedMac(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    NETACCESS_OM_SecureMacEntry_T   new_mac_entry;
    NETACCESS_OM_SecureMacEntry_T   old_mac_entry;
    BOOL_T                          changed = FALSE;

    old_mac_entry.lport = lport;
    memcpy(old_mac_entry.secure_mac, state_machine->port_security_sm.authenticating_mac, SYS_ADPT_MAC_ADDR_LEN);
    if (FALSE == NETACCESS_OM_GetSecureAddressEntry(&old_mac_entry))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("NETACCESS_OM_GetSecureAddressEntry failed");
        return FALSE;
    }

    if (FALSE == NETACCESS_VM_LocalCollectMacEntry(lport, state_machine, &new_mac_entry))
    {
        return FALSE;
    }

    if (1 == state_machine->port_security_sm.event_bitmap.waiting_reauth_result)
    {
        old_mac_entry.holdoff_time = new_mac_entry.holdoff_time;
        old_mac_entry.session_time = new_mac_entry.session_time;
        old_mac_entry.session_expire_time = new_mac_entry.session_expire_time;
        old_mac_entry.server_ip = new_mac_entry.server_ip;
        changed = TRUE;
    }
    else
    {
        if ((1 == new_mac_entry.mac_flag.authorized_by_dot1x) && (1 == old_mac_entry.mac_flag.auth_by_rada))
        {
            if (TRUE == NETACCESS_VM_LocalIsDot1xMaxMacCountReached(lport))
            {
                return FALSE;
            }

            old_mac_entry.mac_flag.eap_packet          = 1;
            old_mac_entry.mac_flag.authorized_by_dot1x = 1;
            old_mac_entry.mac_flag.auth_by_rada        = 0;

            old_mac_entry.session_time = new_mac_entry.session_time;
            old_mac_entry.session_expire_time = new_mac_entry.session_expire_time;
            old_mac_entry.server_ip = new_mac_entry.server_ip;
            changed = TRUE;
        }
    }

    if (TRUE == changed)
    {
        if (FALSE == NETACCESS_OM_UpdateSecureAddressEntryByIndex(&old_mac_entry))
        {
            NETACCESS_VM_DEBUG_PRINT_ERROR("NETACCESS_OM_UpdateSecureAddressEntryByIndex failed");
            return FALSE;
        }
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalEnterAuthenticating
 * ---------------------------------------------------------------------
 * PURPOSE: update state_machine into authenticating status
 * INPUT:  state_machine
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalEnterAuthenticating(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    UI32_T                          debug_flag;
    NETACCESS_OM_PortSecuritySM_T   *psec_sm;
    NETACCESS_NEWMAC_DATA_T         *newmac_data;
    NETACCESS_EAP_DATA_T            *eap_data;

    /* check if input is valid
     */
    if (NULL == state_machine)
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("Null pointer");
        return FALSE;
    }

    debug_flag = NETACCESS_OM_GetDebugFlag();

    psec_sm = &state_machine->port_security_sm;

    /* check if port is authenticating
     */
    if (1 == psec_sm->event_bitmap.is_authenticating)
    {
        /* already enter authenticating status.
         * this could be occurred in second phase authentication
         * (e.g. NETACCESS_PORTMODE_MAC_ADDRESS_OR_USER_LOGIN_SECURE,
         *       NETACCESS_PORTMODE_MAC_ADDRESS_ELSE_USER_LOGIN_SECURE ...)
         */
        return TRUE;
    }

    /* check if port is reAuth
     */
    if (1 == psec_sm->event_bitmap.reauth)
    {
        /* if reauth, no msg will exist
         */
        psec_sm->event_bitmap.is_authenticating = 1;
        return TRUE;
    }

    /* check if there is new MAC msg
     */
    if (NULL == psec_sm->new_mac_msg)
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("new_mac_msg not found");
        return FALSE;
    }

    newmac_data = psec_sm->new_mac_msg->m_newmac_data;
    eap_data    = psec_sm->new_mac_msg->m_eap_data;

    /* check if there is new MAC data or eap data
     */
    if (NULL != newmac_data)
    {
        memcpy(psec_sm->authenticating_mac, newmac_data->src_mac, SYS_ADPT_MAC_ADDR_LEN);
    }
    else if (NULL != eap_data)
    {
        memcpy(psec_sm->authenticating_mac, eap_data->src_mac, SYS_ADPT_MAC_ADDR_LEN);
    }
    else
    {
        /* msg has bad content
         */
        NETACCESS_VM_DEBUG_PRINT_ERROR("msg has bad content");
        return FALSE;
    }

    NETACCESS_VM_DEBUG_PRINT_IMPORTANT(
                              "Authenting MAC is %02X-%02X-%02X-%02X-%02X-%02X",
                              psec_sm->authenticating_mac[0],
                              psec_sm->authenticating_mac[1],
                              psec_sm->authenticating_mac[2],
                              psec_sm->authenticating_mac[3],
                              psec_sm->authenticating_mac[4],
                              psec_sm->authenticating_mac[5]);

    psec_sm->event_bitmap.is_authenticating = 1;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalEnterIdelState
 * ---------------------------------------------------------------------
 * PURPOSE: Enter idel state
 * INPUT:   lport, state_machine
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   none
 * ---------------------------------------------------------------------
 */
static void NETACCESS_VM_LocalEnterIdelState(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{

    if (NULL != state_machine->port_security_sm.cookie)
    {
        NETACCESS_VM_AuthResult_T auth_result;

        auth_result.result = SYS_CALLBACK_MGR_AUTH_FAILED;
        auth_result.vid = 0;

        if (NETACCESS_PORTMODE_NO_RESTRICTIONS == state_machine->running_port_mode)
        {
            auth_result.result = SYS_CALLBACK_MGR_AUTH_BYPASS;
        }
        else
        {
            if (TRUE == NETACCESS_VM_IsInRestrictedVlan(lport))
            {
                auth_result.result = SYS_CALLBACK_MGR_AUTH_BYPASS;
            }
            else
            {
                auth_result.result = SYS_CALLBACK_MGR_AUTH_UNAUTHENTICATED;
            }
        }

        NETACCESS_VM_LocalPassCookieToNextCSC(lport, state_machine, &auth_result);
    }
}

static void NETACCESS_VM_LocalReturnedSeverIP(
    NETACCESS_OM_StateMachine_T *sm_p,
    UI32_T *ip)
{
    if (sm_p->port_security_sm.radius_msg)
    {
        *ip = sm_p->port_security_sm.radius_msg->m_radius_data->server_ip;
    }
    else if (sm_p->port_security_sm.dot1x_msg)
    {
        *ip = sm_p->port_security_sm.dot1x_msg->m_dot1x_data->server_ip;
    }
    else
    {
        *ip = 0;
    }
}


static UI32_T NETACCESS_VM_LocalSessionTime(
    UI32_T lport,
    NETACCESS_OM_StateMachine_T *sm_p
    )
{
    UI32_T sess_time = 0;

    if (sm_p->port_security_sm.radius_msg)
    {
        sess_time = sm_p->port_security_sm.radius_msg->m_radius_data->session_time;
    }
    else if (sm_p->port_security_sm.dot1x_msg)
    {
        sess_time = sm_p->port_security_sm.dot1x_msg->m_dot1x_data->session_time;
        if (0 == sess_time)
        {
            sess_time = NETACCESS_MAX_SESSION_TIME;
        }
    }

    if (sess_time == 0)
    {
    #if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_STATIC)
        sess_time = NETACCESS_MAX_SESSION_TIME;
    #else
        NETACCESS_OM_GetSecureReauthTime(&sess_time);
    #endif
    }

    return sess_time;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_LocalCollectMacEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : collect information for mac_entry
 * INPUT    : state_machine, is_authorized
 * OUTPUT   : mac_entry
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTE     : none
 *-------------------------------------------------------------------------*/
static BOOL_T NETACCESS_VM_LocalCollectMacEntry(
    UI32_T lport,
    NETACCESS_OM_StateMachine_T *state_machine,
    NETACCESS_OM_SecureMacEntry_T *mac_entry
    )
{
    UI32_T      debug_flag = 0;

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* check if input is valid
     */
    if ((NULL == state_machine) || (NULL == mac_entry))
    {
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] input is invalid", __FUNCTION__);

        return FALSE;
    }

    memset(mac_entry, 0, sizeof(NETACCESS_OM_SecureMacEntry_T));

    mac_entry->lport = lport;
    memcpy(mac_entry->secure_mac, state_machine->port_security_sm.authenticating_mac, SYS_ADPT_MAC_ADDR_LEN);
    SYS_TIME_GetRealTimeBySec(&mac_entry->record_time);

    /* don't care is_authorized, always update holdoff_time
     */
    if (FALSE == NETACCESS_OM_GetSecureHoldoffTime(&mac_entry->holdoff_time))
    {
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] get holdoff time fail", __FUNCTION__);
        return FALSE;
    }

    mac_entry->addr_row_status = NETACCESS_ROWSTATUS_ACTIVE;
    mac_entry->authorized_status = NETACCESS_ROWSTATUS_ACTIVE;

    /* get returned information
     */
    if (NULL != state_machine->port_security_sm.radius_msg)
    {
        /* check if msg null
         */
        if (NULL == state_machine->port_security_sm.radius_msg->m_radius_data)
        {
            if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
                printf("\r\n[%s] radius msg data null", __FUNCTION__);
            return FALSE;
        }

        mac_entry->mac_flag.auth_by_rada = 1;
    }
    else if (NULL != state_machine->port_security_sm.dot1x_msg)
    {
        /* check if msg null
         */
        if (NULL == state_machine->port_security_sm.dot1x_msg->m_dot1x_data)
        {
            if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
                printf("\r\n[%s] dot1x msg data null", __FUNCTION__);
            return FALSE;
        }

        /* only one mac SHALL be authorized by 802.1X at any time per port
         */
        mac_entry->mac_flag.authorized_by_dot1x = 1;

        /* if dot1x_msg exists, it means this result passed by dot1x
         */
        mac_entry->mac_flag.eap_packet = 1;
    }

    NETACCESS_VM_LocalReturnedSeverIP(state_machine, &mac_entry->server_ip);

    mac_entry->session_time = NETACCESS_VM_LocalSessionTime(lport, state_machine);
    if (mac_entry->session_time == NETACCESS_MAX_SESSION_TIME)
    {
        mac_entry->session_expire_time = NETACCESS_MAX_SESSION_EXPIRE_TIME;
    }
    else
    {
        mac_entry->session_expire_time = mac_entry->record_time + mac_entry->session_time;
    }

    return TRUE;
}

#if 0
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalGetCurrentVlanString
 * ---------------------------------------------------------------------
 * PURPOSE: collect administrative configured VLAN settings
 * INPUT:  lport
 * OUTPUT: vlan, qos
 * RETURN: TRUE -- succeeded / FALSE -- Failed
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalGetCurrentVlanString(UI32_T lport, UI8_T *vlan_str_p, UI32_T max_str_len)
{
    UI32_T      debug_flag = 0, vid, str_pos, str_len;
    char        vlan_id[NETACCESS_VM_MAX_DIGIT_NBR_OF_VID + 1];
    UI32_T time_mark = 0;
    UI32_T vid_ifindex;
    VLAN_OM_Dot1qVlanCurrentEntry_T  vlan_info;
    VLAN_OM_Dot1qPortVlanEntry_T     port_vlan_entry;

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* check if input is valid
     */
    if (NULL == vlan_str_p)
    {
        return FALSE;
    }

    vlan_str_p[0] = '\0';
    str_pos = vid = 0;

    /* get PVID
     */
    if (VLAN_PMGR_GetDot1qPortVlanEntry(lport ,&port_vlan_entry) == FALSE)
    {
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] VLAN_PMGR_GetDot1qPortVlanEntry(%lu) failed", __FUNCTION__, lport);

        return FALSE;
    }

    /* get next VLAN id to collect administrative VLAN and QoS
     */
    while(VLAN_OM_GetNextDot1qVlanCurrentEntry_forUI(time_mark, &vid, &vlan_info))
    {
        VLAN_OM_ConvertToIfindex(vid, &vid_ifindex);
        if (VLAN_OM_IsPortVlanMember_forUI(vid_ifindex, lport))
        {
            /* insert a comma between VLAN ID
             */
            if (0 != str_pos)
            {
                vlan_str_p[str_pos++] = ' ';
            }

            sprintf(vlan_id, "%lu", vid);
            str_len = strlen(vlan_id);

            /* reserve "1" for tag or untag
             */
            if ((str_pos + str_len + 1) > max_str_len)
            {
                if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
                    printf("\r\n[%s] vlan_assignment is not large enough", __FUNCTION__);
                return FALSE;
            }

            /*strncpy(vlan_str_p[str_pos], vlan_id, NETACCESS_VM_MAX_DIGIT_NBR_OF_VID);*/
            strcat(vlan_str_p, vlan_id);
            str_pos += str_len;

            if (vid == port_vlan_entry.dot1q_pvid_index)
            {
                /* pvid
                 */
                vlan_str_p[str_pos++] = 'p';
            }
            else if (vlan_info.dot1q_vlan_current_untagged_ports[(UI32_T)( (lport - 1)/8 )] & (1 << ( 7 - ( (lport - 1) % 8) ) ) )
            {
                /* untag
                 */
                vlan_str_p[str_pos++] = 'u';
            }
            else
            {
                /* tag
                 */
                vlan_str_p[str_pos++] = 't';
            }
        }
    }

    if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
    {
        printf("\r\n[%s] current vlan_assignment(%s)", __FUNCTION__, vlan_str_p);
    }

    return TRUE;
}
#endif

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalStringState
 * ---------------------------------------------------------------------
 * PURPOSE  : convert state of state machine to string
 * INPUT    : state.
 * OUTPUT   : None.
 * RETURN   : state string
 * NOTES    : none
 * ---------------------------------------------------------------------
 */
static const char* NETACCESS_VM_LocalStringState(NETACCESS_OM_StateMachineStatus_T state)
{
    return (NETACCESS_STATE_SYSTEM_INIT == state) ? "SystemInit" :

           (NETACCESS_STATE_ENTER_SECURE_PORT_MODE == state) ? "EnterSecurePortMode" :
           (NETACCESS_STATE_SECURE_PORT_MODE == state) ? "SecurePortMode" :
           (NETACCESS_STATE_EXIT_SECURE_PORT_MODE == state) ? "ExitSecurePortMode" :

           (NETACCESS_STATE_INIT == state) ? "init" :
           (NETACCESS_STATE_IDLE == state) ? "idle" :
           (NETACCESS_STATE_LEARNING == state) ? "learning" :
           (NETACCESS_STATE_INTRUSION_HANDLING == state) ? "intrusionHandling" :
           (NETACCESS_STATE_AUTHENTICATING == state) ? "authenticating" :
           (NETACCESS_STATE_SUCCEEDED == state) ? "succeeded" :
           (NETACCESS_STATE_FAILED == state) ? "failed" :

           (NETACCESS_STATE_DOT1X_AUTHENTICATING == state) ? "dot1xAuthenticating" :
           (NETACCESS_STATE_RADA_AUTHENTICATING == state) ? "radaAuthenticating" :
           (NETACCESS_STATE_DOT1X_FAILED == state) ? "dot1xFailed" :
           (NETACCESS_STATE_RADA_FAILED == state) ? "radaFailed" :

           (NETACCESS_STATE_PROTO_UNAWARE == state) ? "protUnaware" :
           (NETACCESS_STATE_DISCOVERY == state) ? "discovery" :

           "unknown state";
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalPrintState
 * ---------------------------------------------------------------------
 * PURPOSE: printf state as string
 * INPUT:  state.
 * OUTPUT: None.
 * RETURN: none
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static void NETACCESS_VM_LocalPrintState(NETACCESS_OM_StateMachineStatus_T state)
{
    printf(" (%s) ",
        (NETACCESS_STATE_SYSTEM_INIT == state) ? "SystemInit" :

        (NETACCESS_STATE_ENTER_SECURE_PORT_MODE == state) ? "EnterSecurePortMode" :
        (NETACCESS_STATE_SECURE_PORT_MODE == state) ? "SecurePortMode" :
        (NETACCESS_STATE_EXIT_SECURE_PORT_MODE == state) ? "ExitSecurePortMode" :

        (NETACCESS_STATE_INIT == state) ? "init" :
        (NETACCESS_STATE_IDLE == state) ? "idle" :
        (NETACCESS_STATE_LEARNING == state) ? "learning" :
        (NETACCESS_STATE_INTRUSION_HANDLING == state) ? "intrusionHandling" :
        (NETACCESS_STATE_AUTHENTICATING == state) ? "authenticating" :
        (NETACCESS_STATE_SUCCEEDED == state) ? "succeeded" :
        (NETACCESS_STATE_FAILED == state) ? "failed" :

        (NETACCESS_STATE_DOT1X_AUTHENTICATING == state) ? "dot1xAuthenticating" :
        (NETACCESS_STATE_RADA_AUTHENTICATING == state) ? "radaAuthenticating" :
        (NETACCESS_STATE_DOT1X_FAILED == state) ? "dot1xFailed" :
        (NETACCESS_STATE_RADA_FAILED == state) ? "radaFailed" :

        (NETACCESS_STATE_PROTO_UNAWARE == state) ? "protUnaware" :
        (NETACCESS_STATE_DISCOVERY == state) ? "discovery" :

        "unknown state");
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalPrintMacAddr
 * ---------------------------------------------------------------------
 * PURPOSE: printf mac address as string
 * INPUT:  addr.
 * OUTPUT: None.
 * RETURN: none
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static void NETACCESS_VM_LocalPrintMacAddr(const UI8_T *addr)
{
    if (NULL == addr)
        return;

    printf("%02x-%02x-%02x-%02x-%02x-%02x",
            addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalSendViolationTrap
 * ---------------------------------------------------------------------
 * PURPOSE: Send violation mac trap
 * INPUT:  lport, mac.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE
 * NOTES:  none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalSendViolationTrap(UI32_T lport,UI8_T *mac)
{
    TRAP_EVENT_TrapData_T   trap_data;
    UI32_T  sys_time, holdoff_time;

    /* check if input is valid and get violation trap holdoff time
     */
    if ((NULL == mac) ||
        (FALSE == NETACCESS_OM_GetSecureViolationTrapHoldoffTime(lport, &holdoff_time)))
    {
        return FALSE;
    }

    SYS_TIME_GetRealTimeBySec(&sys_time);

    /* time not expire,hold this trap, don't treat it as an error
     */
    if (holdoff_time >= sys_time)
    {
        return TRUE;
    }

    /* keep violation trap holdoff time
     */
    if (FALSE == NETACCESS_OM_SetSecureViolationTrapHoldoffTime(lport, sys_time + NETACCESS_VM_VIOLATION_TRAP_HOLDOFF_SECONDS))
    {
        return FALSE;
    }

    /* Trap for intrusion mac address */
    trap_data.trap_type = TRAP_EVENT_PORT_SECURITY_TRAP;
    trap_data.community_specified = FALSE;
    trap_data.flag = TRAP_EVENT_SEND_TRAP_OPTION_DEFAULT;
    trap_data.u.port_security_trap.instance_ifindex = lport;
    trap_data.u.port_security_trap.ifindex = lport;

    /* send trap to all community */
    SNMP_PMGR_ReqSendTrap(&trap_data);

    return TRUE;
}

static BOOL_T NETACCESS_VM_HaveOtherUserOnPort(UI32_T lport, NETACCESS_OM_StateMachine_T *sm_p)
{
    UI32_T  user_count = NETACCESS_VM_GetSecureNumberAddressesStoredWithoutMacFilter(lport);

    if (user_count == 0)
        return FALSE;

    if ((user_count == 1) && (1 == sm_p->port_security_sm.event_bitmap.waiting_reauth_result))
        return FALSE;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalIsValidVlanList
 * ---------------------------------------------------------------------
 * PURPOSE: This function validates the VLAN list.
 * INPUT  : lport       -- logic port number
 *          vlan_list_p -- VLAN string list
 * OUTPUT : None.
 * RETURN : TRUE  -- valid
 *          FALSE -- invalid
 * NOTES :  This function checks the port mode and verifies the VLAN list.
 *          After the check. If the VLAN list can be applied to port,
 *          the return_vlan_change flag should be set.
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalIsValidVlanList(UI32_T lport, const char *str, NETACCESS_OM_StateMachine_T *state_machine)
{
    if (NETACCESS_VM_HaveOtherUserOnPort(lport, state_machine))
    {
        UI8_T applied[16] = {0};
        UI8_T returned[16] = {0};

        NETACCESS_OM_GetDynamicVlanMd5(lport, applied);
        PORTAUTHSRVC_MGR_Vlan_StringToMd5(str, returned);

        if (memcmp(applied, returned, sizeof(applied)) != 0)
        {
            NETACCESS_VM_DEBUG_PRINT_ERROR("Returned a different VLAN list");
            return FALSE;
        }
    }

    if (!NETACCESS_VM_LocalIsEmptyStr(str))
    {
        if (!PORTAUTHSRVC_MGR_Vlan_IsValidVlanListString(lport, str))
        {
            NETACCESS_VM_DEBUG_PRINT_ERROR("PORTAUTHSRVC check failed");
            return FALSE;
        }
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalIsValidQosProfiles
 * ---------------------------------------------------------------------
 * PURPOSE: This function validates the QoS profiles string that returns
 *          from RADIUS server.
 * INPUT  : lport   -- logic port number
 *          string  -- QoS profiles string
 *          sm_p    -- state machine
 * OUTPUT : None.
 * RETURN : TRUE  -- valid
 *          FALSE -- invalid
 * NOTES :  This function checks the port mode and verifies the QoS profiles.
 *          After the check. If the 802.1p profile can be applied to port,
 *          the return_default_port_priority_changed flag should be set.
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalIsValidQosProfiles(
    UI32_T lport,
    const char *str,
    NETACCESS_OM_StateMachine_T *sm_p
    )
{
    if (NETACCESS_VM_HaveOtherUserOnPort(lport, sm_p))
    {
        UI8_T applied[16] = {0};
        UI8_T returned[16] = {0};

        NETACCESS_OM_GetDynamicQosProfileMd5(lport, applied);
        PORTAUTHSRVC_MGR_Qos_StringToMd5(str, returned);

        if (memcmp(applied, returned, sizeof(applied)) != 0)
        {
            NETACCESS_VM_DEBUG_PRINT_ERROR("Returned a different QoS profile");
            return FALSE;
        }
    }

    if (!NETACCESS_VM_LocalIsEmptyStr(str))
    {
        if (FALSE == PORTAUTHSRVC_MGR_Qos_IsValidProfileString(lport, str))
        {
            NETACCESS_VM_DEBUG_PRINT_ERROR("PORTAUTHSRVC check failed");
            return FALSE;
        }
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_LocalApplyQosProfiles
 *-------------------------------------------------------------------------
 * PURPOSE  : Apply the QoS profiles to the specified port
 * INPUT    : lport - lport index
 *            str   - qos profiles string
 *            sm_p  - state machine
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTE     :
 *-------------------------------------------------------------------------*/
static BOOL_T NETACCESS_VM_LocalApplyQosProfiles(
    UI32_T lport,
    const char *str,
    NETACCESS_OM_StateMachine_T *sm_p
    )
{
    BOOL_T enabled;

    if (NULL == str || NULL == sm_p)
    {
        return FALSE;
    }

    if (FALSE == NETACCESS_OM_GetDynamicQosStatus(lport, &enabled))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("Wrong lport(%lu)", lport);
        return FALSE;
    }

    if (!enabled)
    {
        return TRUE;
    }

    {
        UI8_T applied[16] = {0};
        UI8_T returned[16] = {0};

        NETACCESS_OM_GetDynamicQosProfileMd5(lport, applied);
        PORTAUTHSRVC_MGR_Qos_StringToMd5(str, returned);

        if (memcmp(applied, returned, sizeof(applied)) != 0)
        {
            UI8_T null_md5[16] = {0};

            PORTAUTHSRVC_MGR_Qos_SetToAdmin(lport);
            NETACCESS_OM_SetDynamicQosProfileMd5(lport, null_md5);

            if (!NETACCESS_VM_LocalIsEmptyStr(str))
            {
                if (!PORTAUTHSRVC_MGR_Qos_SetToOper(lport, str))
                {
                    NETACCESS_VM_DEBUG_PRINT_ERROR("PORTAUTHSRVC apply failed");
                    return FALSE;
                }
            }

            NETACCESS_OM_SetDynamicQosProfileMd5(lport, returned);
        }
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_LocalResetQos2Default
 *-------------------------------------------------------------------------
 * PURPOSE  : Reset QoS to administrative configuration
 * INPUT    : lport
 * OUTPUT   : None
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTE     : none
 *-------------------------------------------------------------------------*/
static BOOL_T NETACCESS_VM_LocalResetQos2Default(UI32_T lport)
{
    BOOL_T ret = TRUE;
    UI8_T  applied_md5[16] = {0};
    UI8_T  null_md5[16] = {0};

    NETACCESS_OM_GetDynamicQosProfileMd5(lport, applied_md5);
    if (memcmp(applied_md5, null_md5, sizeof(applied_md5)) != 0)
    {
        ret = PORTAUTHSRVC_MGR_Qos_SetToAdmin(lport);

        if (!ret)
        {
            NETACCESS_VM_DEBUG_PRINT_ERROR("PORTAUTHSRVC_MGR_Qos_SetToAdmin failed");
        }

        NETACCESS_OM_SetDynamicQosProfileMd5(lport, null_md5);
    }

    return ret;
}

static void NETACCESS_VM_LocalInitializeFunctionPointer(void)
{
    UI32_T port_mode;

    memset(port_mode_fun_p, 0, sizeof(NETACCESS_VM_PortModeFuncPtr_T)*(NETACCESS_PORTMODE_MAX-1));

    /* noRestrictions
     */
    port_mode = NETACCESS_PORTMODE_NO_RESTRICTIONS;
    port_mode_fun_p[port_mode-1].allow_vlan_change =  NULL;
    port_mode_fun_p[port_mode-1].allow_reauth = NULL;
    port_mode_fun_p[port_mode-1].enter_port_mode = (NETACCESS_VM_EnterPortModeFuncPtr_T)NETACCESS_VM_LocalEnterPortModeEnableAutoLearn;

    /* secure
     */
    port_mode = NETACCESS_PORTMODE_SECURE;
    port_mode_fun_p[port_mode-1].allow_vlan_change = NULL;
    port_mode_fun_p[port_mode-1].allow_reauth = NULL;
    port_mode_fun_p[port_mode-1].enter_port_mode = (NETACCESS_VM_EnterPortModeFuncPtr_T)NETACCESS_VM_LocalEnterSecureMode;

    /* macAddressORuserLoginSecure
     */
    port_mode = NETACCESS_PORTMODE_MAC_ADDRESS_OR_USER_LOGIN_SECURE;
    port_mode_fun_p[port_mode-1].allow_vlan_change = NULL;
    port_mode_fun_p[port_mode-1].allow_reauth = NULL;
    port_mode_fun_p[port_mode-1].enter_port_mode = (NETACCESS_VM_EnterPortModeFuncPtr_T)NETACCESS_VM_LocalEnterMacAddressOrUserSecureMode;


    /* macAuthentication
     */
    port_mode = NETACCESS_PORTMODE_MAC_AUTHENTICATION;
    port_mode_fun_p[port_mode-1].allow_vlan_change = (NETACCESS_VM_AllowVlanChangeFuncPtr_T)NETACCESS_VM_LocalIsPortModeAllowVlanChange;
    port_mode_fun_p[port_mode-1].allow_reauth = NULL;
    port_mode_fun_p[port_mode-1].enter_port_mode = (NETACCESS_VM_EnterPortModeFuncPtr_T)NETACCESS_VM_LocalEnterMacAuthenticationMode;

    /* portSecurity
     */
    port_mode = NETACCESS_PORTMODE_PORT_SECURITY;
    port_mode_fun_p[port_mode-1].allow_vlan_change = NULL;
    port_mode_fun_p[port_mode-1].allow_reauth = NULL;
    port_mode_fun_p[port_mode-1].enter_port_mode = (NETACCESS_VM_EnterPortModeFuncPtr_T)NETACCESS_VM_LocalEnterPortSecurityMode;

    /* dot1x
     */
    port_mode = NETACCESS_PORTMODE_DOT1X;
    port_mode_fun_p[port_mode-1].allow_vlan_change = NULL;
    port_mode_fun_p[port_mode-1].allow_reauth = NULL;
    port_mode_fun_p[port_mode-1].enter_port_mode = (NETACCESS_VM_EnterPortModeFuncPtr_T)NETACCESS_VM_LocalEnterDot1xMode;


    /* link status change handlers
     */
    memset(&netaccess_vm_port_link_up_handler, 0, sizeof(netaccess_vm_port_link_up_handler));
    memset(&netaccess_vm_port_link_down_handler, 0, sizeof(netaccess_vm_port_link_down_handler));

#if (SYS_CPNT_DOT1X == TRUE)
    netaccess_vm_port_link_up_handler[NETACCESS_VM_DOT1X] = NETACCESS_VM_Dot1x_ProcessLinkUpEvent;
    netaccess_vm_port_link_down_handler[NETACCESS_VM_DOT1X] = NETACCESS_VM_Dot1x_ProcessLinkDownEvent;
#endif

    netaccess_vm_port_link_down_handler[NETACCESS_VM_RUNNING_PORT_MODE] = NETACCESS_VM_ProcessLinkDown;

#if (SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
    netaccess_vm_port_link_up_handler[NETACCESS_VM_LINK_DETECTION] = NETACCESS_VM_LinkDetection_ProcessLinkUpEvent;
    netaccess_vm_port_link_down_handler[NETACCESS_VM_LINK_DETECTION] = NETACCESS_VM_LinkDetection_ProcessLinkDownEvent;
#endif

    return;
}

static BOOL_T NETACCESS_VM_LocalIsPortModeAllowVlanChange(UI32_T lport)
{
    BOOL_T dynamic_vlan_status;
    UI32_T  debug_flag = 0;

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* get dynamic vlan status
     */
    if (FALSE == NETACCESS_OM_GetDynamicVlanStatus(lport, &dynamic_vlan_status))
    {
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] get dynamic vlan status fail,not allow vlan change", __FUNCTION__);

        return FALSE;
    }

    /* if dynamic vlan disable,not allow vlan change
     */
    if (FALSE == dynamic_vlan_status)
    {
        if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
            printf("\r\n[%s] dynamic vlan disable,not allow vlan change", __FUNCTION__);

        return FALSE;
    }

    return TRUE;
}

static BOOL_T NETACCESS_VM_LocalIsPortAllowVlanChange(UI32_T lport)
{
    UI32_T      running_port_mode;
    UI32_T  debug_flag = 0;

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* get dynamic VLAN status and port mode
     */
    if (FALSE == NETACCESS_OM_GetPortStateMachineRunningPortMode(lport, &running_port_mode))
    {
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] get running port mode fail", __FUNCTION__);

        return FALSE;
    }

    /* call function pointer
     */
    if(NULL != port_mode_fun_p[running_port_mode-1].allow_vlan_change)
    {
        return port_mode_fun_p[running_port_mode-1].allow_vlan_change(lport);
    }

    if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
        printf("\r\n[%s] null function pointer,not allow vlan change", __FUNCTION__);
    return FALSE;
}

static BOOL_T NETACCESS_VM_LocalIsSmAllowAddressReauth(NETACCESS_OM_StateMachine_T *state_machine, UI32_T mac_index)
{
    UI32_T port_mode;

    port_mode = state_machine->running_port_mode;

    /* run function pointer
     */
    if(NULL != port_mode_fun_p[port_mode-1].allow_reauth)
    {
        return port_mode_fun_p[port_mode-1].allow_reauth(state_machine, mac_index);
    }

    return FALSE;
}

static BOOL_T NETACCESS_VM_LocalEnterPortModeEnableAutoLearn(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    UI32_T      debug_flag = 0;

    debug_flag = NETACCESS_OM_GetDebugFlag();

    if (NETACCESS_OM_DEBUG_VM_TRC & debug_flag)
        printf("\r\n[%s] start processing", __FUNCTION__);

    return NETACCESS_VM_LocalEnableAutoLearning(lport);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalEnterMacAuthenticationMode
 * ---------------------------------------------------------------------
 * PURPOSE: Do thing for enter MAC authentication mode
 * INPUT  : lport, state_machine
 * OUTPUT : none.
 * RETURN : TRUE - succeeded; FALSE - failed
 * NOTES  : none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalEnterMacAuthenticationMode(
    UI32_T lport,
    NETACCESS_OM_StateMachine_T *state_machine
    )
{
    UI32_T      debug_flag = 0;

    NETACCESS_VM_DEBUG_PRINT_FUNCTION_START();

    debug_flag = NETACCESS_OM_GetDebugFlag();

    /* disable auto learn
     */
    if (FALSE == NETACCESS_VM_LocalDisableAutoLearningAndTrap2CPU(lport, NETACCESS_PORTMODE_MAC_AUTHENTICATION))
    {
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] disable auto learn failed", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalEnterPortSecurityMode
 * ---------------------------------------------------------------------
 * PURPOSE: Do thing for enter port security mode
 * INPUT  : lport, state_machine
 * OUTPUT : none.
 * RETURN : TRUE - succeeded; FALSE - failed
 * NOTES  : none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalEnterPortSecurityMode(
    UI32_T lport,
    NETACCESS_OM_StateMachine_T *state_machine
    )
{
    UI32_T      debug_flag = 0;

    NETACCESS_VM_DEBUG_PRINT_FUNCTION_START();

    debug_flag = NETACCESS_OM_GetDebugFlag();

    if (FALSE == NETACCESS_VM_LocalDisableAutoLearningAndTrap2CPU(lport, NETACCESS_PORTMODE_PORT_SECURITY))
    {
        if (NETACCESS_OM_DEBUG_VM_ERR & debug_flag)
            printf("\r\n[%s] disable auto learn failed", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalEnterSecureMode
 * ---------------------------------------------------------------------
 * PURPOSE: do thing for enter secure mode
 * INPUT  : lport, state_machine
 * OUTPUT : none.
 * RETURN : TRUE - succeeded; FALSE - failed
 * NOTES  : none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalEnterSecureMode(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    NETACCESS_VM_DEBUG_PRINT_FUNCTION_START();

    if (FALSE == NETACCESS_VM_LocalDisableAutoLearningAndTrap2CPU(lport, NETACCESS_PORTMODE_SECURE))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("Enable secure mode failed");
        return FALSE;
    }
    return TRUE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalEnterMacAddressOrUserSecureMode
 * ---------------------------------------------------------------------
 * PURPOSE: do thing for enter macAddressOrUserSecure mode
 * INPUT  : lport, state_machine
 * OUTPUT : none.
 * RETURN : TRUE - succeeded; FALSE - failed
 * NOTES  : none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalEnterMacAddressOrUserSecureMode(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
    NETACCESS_VM_DEBUG_PRINT_FUNCTION_START();

    if (FALSE == NETACCESS_VM_LocalDisableAutoLearningAndTrap2CPU(lport, NETACCESS_PORTMODE_MAC_ADDRESS_OR_USER_LOGIN_SECURE))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("Enable macAddressOrUserSecure mode failed");
        return FALSE;
    }
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalEnterDot1xMode
 * ---------------------------------------------------------------------
 * PURPOSE: do thing for enter dot1x mode
 * INPUT  : lport, state_machine
 * OUTPUT : none.
 * RETURN : TRUE - succeeded; FALSE - failed
 * NOTES  : none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalEnterDot1xMode(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
#if (SYS_CPNT_DOT1X == TRUE)
    RULE_TYPE_CpuRuleInfo_T rule_info;
    UI8_T                   cpu_mac[SYS_ADPT_MAC_ADDR_LEN];

    NETACCESS_VM_DEBUG_PRINT_FUNCTION_START();

    if (FALSE == NETACCESS_VM_LocalDisableAutoLearningAndTrap2CPU(lport, NETACCESS_PORTMODE_DOT1X))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("NETACCESS_VM_LocalDisableAutoLearningAndTrap2CPU failed");
        return FALSE;
    }

    SWCTRL_GetCpuMac(cpu_mac);

    memcpy(rule_info.dot1x.cpu_mac, cpu_mac, SYS_ADPT_MAC_ADDR_LEN);
    rule_info.dot1x.ifindex = lport;
    if (FALSE == L4_MGR_TrapPacket2Cpu(TRUE, RULE_TYPE_PacketType_DOT1X, &rule_info))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("L4_MGR_TrapPacket2Cpu failed");
        /*return FALSE;*/
    }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalExitDot1xMode
 * ---------------------------------------------------------------------
 * PURPOSE: do thing for exit dot1x mode
 * INPUT  : lport, state_machine
 * OUTPUT : none.
 * RETURN : TRUE - succeeded; FALSE - failed
 * NOTES  : none
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalExitDot1xMode(UI32_T lport, NETACCESS_OM_StateMachine_T *state_machine)
{
#if (SYS_CPNT_DOT1X == TRUE)
    RULE_TYPE_CpuRuleInfo_T rule_info;
    UI8_T                   cpu_mac[SYS_ADPT_MAC_ADDR_LEN];

    NETACCESS_VM_DEBUG_PRINT_FUNCTION_START();

    SWCTRL_GetCpuMac(cpu_mac);

    memcpy(rule_info.dot1x.cpu_mac, cpu_mac, SYS_ADPT_MAC_ADDR_LEN);
    rule_info.dot1x.ifindex = lport;
    if (FALSE == L4_MGR_TrapPacket2Cpu(FALSE, RULE_TYPE_PacketType_DOT1X, &rule_info))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("L4_MGR_TrapPacket2Cpu failed");
        /*return FALSE;*/
    }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalIsVoiceVlanEnabled
 * ---------------------------------------------------------------------
 * PURPOSE: Is voice VLAN enabled on the specified port
 * INPUT  : lport
 * OUTPUT : None
 * RETURN : TRUE - enabled; FALSE - disabled
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalIsVoiceVlanEnabled(UI32_T lport)
{
#if (SYS_CPNT_ADD == TRUE)
    UI32_T add_port_mode;

    if (   (TRUE == ADD_OM_IsVoiceVlanEnabled())
        && (TRUE == ADD_OM_GetVoiceVlanPortMode(lport, &add_port_mode))
        && (VAL_voiceVlanPortMode_none != add_port_mode)
       )
    {
        return TRUE;
    }
#endif

    return FALSE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalIsVoiceVlanId
 * ---------------------------------------------------------------------
 * PURPOSE: Is VLAN ID assigned as voice VLAN ID
 * INPUT  : vid
 * OUTPUT : None
 * RETURN : TRUE - yes; FALSE - no
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalIsVoiceVlanId(UI32_T vid)
{
#if (SYS_CPNT_ADD == TRUE)
    I32_T vvid;
    if (   (TRUE == ADD_OM_IsVoiceVlanEnabled())
        && (TRUE == ADD_OM_GetVoiceVlanId(&vvid))
        && (vvid == vid)
       )
    {
        return TRUE;
    }
#endif

    return FALSE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalByPassToVoiceVlan
 * ---------------------------------------------------------------------
 * PURPOSE: Bypass to do voice VLAN authentication
 * INPUT  : vid, mac, lport
 * OUTPUT : None
 * RETURN : TRUE - succeeded; FALSE - failed
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalByPassToVoiceVlan(UI32_T vid, UI8_T *mac, UI32_T lport)
{
#if (SYS_CPNT_ADD == TRUE)
    UI32_T  vid_ifidx;
    UI8_T   pri;

    AMTR_TYPE_AddrEntry_T entry = {0};

    /* voice VLAN authentication
     */
    if (TRUE == ADD_MGR_ProcessNewMac(vid, mac, lport))
    {
        /* remap to new priority
         */
        ADD_MGR_GetVoiceVlanPortPriority(lport, &pri);
        entry.vid = vid;
        memcpy(entry.mac, mac, SYS_ADPT_MAC_ADDR_LEN);
        entry.ifindex = lport;

        entry.action = AMTR_TYPE_ADDRESS_ACTION_FORWARDING;
        entry.source = AMTR_TYPE_ADDRESS_SOURCE_LEARN;
        entry.life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT;
        entry.priority = pri;

        return AMTR_MGR_SetAddrEntryWithPriority(&entry);


        //return AMTR_MGR_SetAddrEntryWithPriority(vid, mac, lport, AMTR_MGR_ENTRY_ATTRIBUTE_LEARNT, pri);
    }
    else
    {
        /* set drop attribute if failed to pass the authentication
         */
        VLAN_OM_ConvertToIfindex(vid, &vid_ifidx);
        if (TRUE == VLAN_OM_IsPortVlanMember(vid_ifidx, lport))
        {

            entry.vid = vid;
            memcpy(entry.mac, mac, SYS_ADPT_MAC_ADDR_LEN);
            entry.ifindex = lport;

            entry.action = AMTR_TYPE_ADDRESS_ACTION_DISCARD_BY_SA_MATCH;
            entry.source = AMTR_TYPE_ADDRESS_SOURCE_LEARN;
            entry.life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT;
            //entry.priority = pri;

            return AMTR_MGR_SetAddrEntryWithPriority(&entry);


            //return AMTR_MGR_SetFilteringAddrrEntry(vid, mac, lport, AMTR_MGR_ENTRY_ATTRIBUTE_LEARNT_DROP);
        }
    }
#endif
    return FALSE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_LocalDeleteSecureMacAddr
 *-------------------------------------------------------------------------
 * PURPOSE  : Delete a secure MAC address from AMTR & OM.
 * INPUT    : lport
 * OUTPUT   : None
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTE     : The MAC address should exist in HISAM befor call this function.
 *            If the secure MAC is authorized by dot1x, this function will
 *            trigger dot1x state machine to send EAPOL packet.
 *-------------------------------------------------------------------------*/
static BOOL_T NETACCESS_VM_LocalDeleteSecureMacAddr(NETACCESS_OM_SecureMacEntry_T *mac_entry_p)
{
    if (NULL == mac_entry_p)
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("Null pointer");
        return FALSE;
    }

    /* Delete secure MAC address from OM & hisam
     */
    if (FALSE == NETACCESS_OM_DeleteSecureAddressEntryByIndex(mac_entry_p->mac_index))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("NETACCESS_OM_DeleteSecureAddressEntryByIndex failed");
        return FALSE;
    }

    /* if mac had been add to AMTR, delete from AMTR
     */
    //if (mac_entry_p->vlan_counter > 0)
    {
        /* for each joined VLAN on the specified port to delete the specified MAC address from AMTR
         */
        if (FALSE == NETACCESS_VM_DeleteMacFromAmtrByPort(mac_entry_p->lport, mac_entry_p->secure_mac))
        {
            NETACCESS_VM_DEBUG_PRINT_ERROR("NETACCESS_VM_DeleteMacFromAmtrByPort failed");
            return FALSE;
        }
    }

    /* if deleted mac is authorized by dot1x, it need to send eap fail packet.
     * if remainder mac are not authorized by dot1x, clear dot1x flag.
     */
    if (1 == mac_entry_p->mac_flag.authorized_by_dot1x)
    {
#if 0
        /* notify dot1x send EAP-Failure packet to supplicant
         */
        if (FALSE == NETACCESS_VM_LocalNotifyDot1xSendEapResult(mac_entry_p->lport, mac_entry_p->secure_mac, FALSE))
        {
            NETACCESS_VM_DEBUG_PRINT_ERROR("NETACCESS_VM_LocalNotifyDot1xSendEapResult failed");
        }
#endif /* #if 0 */

        /* if no any mac is authorized by dot1x, clear dot1x flag
         */
        if(FALSE == NETACCESS_VM_LocalHaveDot1xMac(mac_entry_p->lport))
        {
            if (FALSE == NETACCESS_OM_ClearStateMachineDot1xLogonFlag(mac_entry_p->lport))
            {
                NETACCESS_VM_DEBUG_PRINT_ERROR("NETACCESS_OM_ClearStateMachineDot1xLogonFlag failed");
                return FALSE;
            }
        }
    }

    return TRUE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_VM_SecureAddrSourceType
 *------------------------------------------------------------------------
 * FUNCTION: Get the source type of secure MAC address
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static UI32_T NETACCESS_VM_SecureAddrSourceType()
{
    return AMTR_TYPE_ADDRESS_SOURCE_SECURITY;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_VM_LocalAgingModeOfSecureAddress
 *------------------------------------------------------------------------
 * FUNCTION: Is enabled aging mode of secure address
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE - Enable / FALSE - Disable
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalAgingModeOfSecureAddress()
{
    BOOL_T enable = FALSE;

#ifdef SYS_CPNT_NETACCESS_AGING_MODE

#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_DYNAMIC)
    enable = TRUE;
#elif (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_STATIC)
    enable = FALSE;
#elif (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)
    {
        UI32_T mode;

        NETACCESS_OM_GetMacAddressAgingMode(&mode);

        if (VAL_networkAccessAging_enabled == mode)
            enable = TRUE;
        else
            enable = FALSE;
    }
#endif

#endif /* #ifdef SYS_CPNT_NETACCESS_AGING_MODE */

    return enable;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_VM_SecureAddrLifeTime
 *------------------------------------------------------------------------
 * FUNCTION: Get the life time of secure MAC address
 * INPUT   : aging - the aging mode of secure address
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static AMTR_TYPE_AddressLifeTime_T NETACCESS_VM_SecureAddrLifeTime(BOOL_T sec_aging)
{
    AMTR_TYPE_AddressLifeTime_T life_time =
        (sec_aging == TRUE) ? AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT :
        AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET;

    NETACCESS_VM_DEBUG_PRINT_TRACE("Life Time = %s",
        (life_time == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT?"delete-on-timeout":
        life_time == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET ? "delete-on-reset":"other"));

    return life_time;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_GetSecureNumberAddressesStoredAuthorByRada
 *-------------------------------------------------------------------------
 * PURPOSE  : get the stored secure number of address that add by which port mode
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : the stored secure number of address
 * NOTE     : None
 *-------------------------------------------------------------------------*/
UI32_T NETACCESS_VM_GetSecureNumberAddressesStoredAuthorByRada(UI32_T lport)
{
    UI32_T  count;
    NETACCESS_OM_SecureMacEntry_T  mac_entry;

    memset(&mac_entry, 0, sizeof(NETACCESS_OM_SecureMacEntry_T));
    mac_entry.lport = lport;

    /* sort by interface
     */
    count = 0;
    while(NETACCESS_OM_GetNextSecureAddressEntry(&mac_entry))
    {
        if (mac_entry.lport != lport)
        {
            break;
        }

        if (1 == mac_entry.mac_flag.auth_by_rada)
        {
            count++;
        }
    }

    return count;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_GetSecureNumberAddressesStoredWithoutMacFilter
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the stored secure number of address without MAC filter
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : the stored secure number of address
 * NOTE     : None
 *-------------------------------------------------------------------------*/
UI32_T NETACCESS_VM_GetSecureNumberAddressesStoredWithoutMacFilter(UI32_T lport)
{
    UI32_T  count;
    NETACCESS_OM_SecureMacEntry_T  mac_entry;

    memset(&mac_entry, 0, sizeof(NETACCESS_OM_SecureMacEntry_T));
    mac_entry.lport = lport;

    /* sort by interface
     */
    count = 0;
    while(NETACCESS_OM_GetNextSecureAddressEntry(&mac_entry))
    {
        if (mac_entry.lport != lport)
        {
            break;
        }

        if (0 == mac_entry.mac_flag.is_mac_filter_mac)
        {
            count++;
        }
    }

    return count;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_VM_NotifyDot1xManagerSetupResult
 * ---------------------------------------------------------------------
 * PURPOSE : Notify dot1x that manager setup's result
 * INPUT   : lport - port number
 *           success - manager setup result is success or not
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_NotifyDot1xManagerSetupResult(
    UI32_T lport,
    NETACCESS_OM_StateMachine_T *sm_p,
    BOOL_T success
    )
{
#if (SYS_CPNT_DOT1X == TRUE)
    DOT1X_SM_AUTH_Obj_T *dot1x_sm_p = DOT1X_OM_GetSMObjByPortMac(lport, sm_p->port_security_sm.authenticating_mac);

    if (NULL == dot1x_sm_p)
    {
        return FALSE;
    }
    if (TRUE == success)
    {
        DOT1X_SM_AUTH_Go(dot1x_sm_p, DOT1X_SM_AUTH_MANAGER_SETUP_SUCC_EV);
    }
    else
    {
        DOT1X_SM_AUTH_Go(dot1x_sm_p, DOT1X_SM_AUTH_MANAGER_SETUP_FAIL_EV);
    }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessLinkDown
 * ---------------------------------------------------------------------
 * PURPOSE: Process for link down event by current port mode
 * INPUT  : lport
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
static void NETACCESS_VM_ProcessLinkDown(UI32_T lport)
{
    NETACCESS_PortMode_T port_mode;

    if (FALSE == NETACCESS_OM_GetSecurePortMode(lport, &port_mode))
    {
        return;
    }

    if (NETACCESS_PORTMODE_NO_RESTRICTIONS == port_mode)
    {
        return;
    }

    NETACCESS_VM_LocalPortDown(lport);
}

#if (SYS_CPNT_DOT1X == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_Dot1x_ProcessLinkUpEvent
 * ---------------------------------------------------------------------
 * PURPOSE: Dot1X, process for link up event.
 * INPUT  : lport
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
static void NETACCESS_VM_Dot1x_ProcessLinkUpEvent(UI32_T lport)
{
    UI32_T unit, port, trunk_id;

   /* check if port is valid
    */
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        return;
    }

    DOT1X_VM_NotifyPortLinkUp(unit, port);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_Dot1x_ProcessLinkDownEvent
 * ---------------------------------------------------------------------
 * PURPOSE: Dot1X, process for link down event.
 * INPUT  : lport
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
static void NETACCESS_VM_Dot1x_ProcessLinkDownEvent(UI32_T lport)
{
    UI32_T unit, port, trunk_id;

     /* check if port is valid
     */
    if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        return;
    }

    DOT1X_VM_NotifyPortLinkDown(unit, port);
}
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

#if (SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LinkDetection_ProcessLinkUpEvent
 * ---------------------------------------------------------------------
 * PURPOSE: Link Detection, process for link up event.
 * INPUT  : lport
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
static void NETACCESS_VM_LinkDetection_ProcessLinkUpEvent(UI32_T lport)
{
    UI32_T  link_detection_status;
    UI32_T  link_detection_mode;
    UI32_T  link_detection_action;

    if (  (FALSE == NETACCESS_OM_GetLinkDetectionStatus(lport, &link_detection_status))
        ||(FALSE == NETACCESS_OM_GetLinkDetectionMode(lport, &link_detection_mode))
        ||(FALSE == NETACCESS_OM_GetLinkDetectionAction(lport, &link_detection_action))
       )
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("Cannot get link-detection setting on port(%lu)", lport);
        return;
    }

    if (VAL_networkAccessPortLinkDetectionStatus_disabled == link_detection_status)
    {
        return;
    }

    if (VAL_networkAccessPortLinkDetectionMode_linkDown == link_detection_mode)
    {
        return;
    }

    NETACCESS_VM_LinkDetection_DoAction(lport, VAL_ifOperStatus_up, link_detection_mode, link_detection_action);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LinkDetection_ProcessLinkDownEvent
 * ---------------------------------------------------------------------
 * PURPOSE: Link Detection, process for link down event.
 * INPUT  : lport
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
static void NETACCESS_VM_LinkDetection_ProcessLinkDownEvent(UI32_T lport)
{
    UI32_T  link_detection_status;
    UI32_T  link_detection_mode;
    UI32_T  link_detection_action;


    if (  (FALSE == NETACCESS_OM_GetLinkDetectionStatus(lport, &link_detection_status))
        ||(FALSE == NETACCESS_OM_GetLinkDetectionMode(lport, &link_detection_mode))
        ||(FALSE == NETACCESS_OM_GetLinkDetectionAction(lport, &link_detection_action))
       )
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("Cannot get link-detection setting on port(%lu)", lport);
        return;
    }

    if (VAL_networkAccessPortLinkDetectionStatus_disabled == link_detection_status)
    {
        return;
    }

    if (VAL_networkAccessPortLinkDetectionMode_linkUp == link_detection_mode)
    {
        return;
    }

    NETACCESS_VM_LinkDetection_DoAction(lport, VAL_ifOperStatus_down, link_detection_mode, link_detection_action);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LinkDetection_DoAction
 * ---------------------------------------------------------------------
 * PURPOSE: Do link detection action
 * INPUT  : lport, oper+status link_detection_mode, link_detection_action
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
static void NETACCESS_VM_LinkDetection_DoAction(UI32_T lport, UI32_T oper_status, UI32_T link_detection_mode, UI32_T link_detection_action)
{
    if (  (VAL_networkAccessPortLinkDetectionAciton_trap == link_detection_action)
      ||(VAL_networkAccessPortLinkDetectionAciton_trapAndShutdown == link_detection_action)
        )
    {
        NETACCESS_VM_LinkDetection_SendTrap(lport, oper_status, link_detection_mode, link_detection_action);
    }

    if (  (VAL_networkAccessPortLinkDetectionAciton_shutdown == link_detection_action)
        ||(VAL_networkAccessPortLinkDetectionAciton_trapAndShutdown == link_detection_action)
        )
    {
        NETACCESS_VM_LinkDetection_Shutdown(lport);
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LinkDetection_SendTrap
 * ---------------------------------------------------------------------
 * PURPOSE: Send link detection trap
 * INPUT  : lport, oper_status, link_detection_mode, link_detection_action
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
static void NETACCESS_VM_LinkDetection_SendTrap(UI32_T lport, UI32_T oper_status, UI32_T link_detection_mode, UI32_T link_detection_action)
{
    TRAP_EVENT_TrapData_T     trap_data;

    trap_data.trap_type = TRAP_EVENT_NETWORK_ACCESS_PORT_LINK_DETECTION_TRAP;
    trap_data.community_specified = FALSE;
    trap_data.flag = TRAP_EVENT_SEND_TRAP_OPTION_DEFAULT;

    trap_data.u.network_access_port_link_detection.instance_ifindex = lport;
    trap_data.u.network_access_port_link_detection.ifindex = lport;
    trap_data.u.network_access_port_link_detection.instance_ifOperStatus = lport;
    trap_data.u.network_access_port_link_detection.ifOperStatus = oper_status;
    trap_data.u.network_access_port_link_detection.instance_networkAccessPortLinkDetectionMode = lport;
    trap_data.u.network_access_port_link_detection.networkAccessPortLinkDetectionMode = link_detection_mode;
    trap_data.u.network_access_port_link_detection.instance_networkAccessPortLinkDetectionAction = lport;
    trap_data.u.network_access_port_link_detection.networkAccessPortLinkDetectionAction = link_detection_action;

    NETACCESS_VM_DEBUG_PRINT_IMPORTANT(
                  "Send a link-detection trap, port(%lu) mode(%lu) action(%lu)",
                  lport, link_detection_mode, link_detection_action);
    SNMP_PMGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
}

static void NETACCESS_VM_LinkDetection_Shutdown(UI32_T lport)
{
    NETACCESS_VM_DEBUG_PRINT_IMPORTANT("Shutdown port, port(%lu)", lport);

    if (FALSE == SYS_CALLBACK_MGR_SetPortStatusCallback(SYS_MODULE_NETACCESS, lport, FALSE, SWCTRL_PORT_STATUS_SET_BY_NETACCESS_LINK_DETECTION))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("SYS_CALLBACK_MGR_SetPortStatusCallback(%lu, down) failed", lport);
    }
}

#endif /* #if (SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE) */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_IsMacFilterMatched
 * ---------------------------------------------------------------------
 * PURPOSE: Match the MAC address with each entry of MAC filter table
 * INPUT  : mac, lport
 * OUTPUT : None
 * RETURN : TRUE - Matched; FALSE - No match
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_IsMacFilterMatched(UI32_T lport, UI8_T *mac)
{
#if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE)
    UI32_T filter_id;
    UI8_T  filter_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T  filter_mask[SYS_ADPT_MAC_ADDR_LEN];

    if((TRUE == NETACCESS_OM_GetApplyMacFilterByPort(lport, &filter_id)) &&
        (0 != filter_id))
    {
        memset(filter_mac, 0, sizeof(filter_mac));
        memset(filter_mask, 0, sizeof(filter_mask));
        while (TRUE == NETACCESS_OM_GetNextMacFilterByFilterId(filter_id, filter_mac, filter_mask))
        {
            if (TRUE == NETACCESS_OM_IsMacFilterMatched(filter_mac, filter_mask, mac))
            {
                return TRUE;
            }
        }
    }
#endif /*#if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE)*/
    return FALSE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalPassCookieToNextCSC
 * ---------------------------------------------------------------------
 * PURPOSE: To transmit cookie to next CSC.
 * INPUT  : lport -- port number
 *          state_machine_p -- status of running state
 *          auth_result_p -- authentication result
 * OUTPUT : None.
 * RETURN : None.
 * NOTES :
 * ---------------------------------------------------------------------
 */
static void NETACCESS_VM_LocalPassCookieToNextCSC(
    UI32_T lport,
    NETACCESS_OM_StateMachine_T *state_machine_p,
    NETACCESS_VM_AuthResult_T *auth_result_p)
{

    NETACCESS_VM_LocalUpdateAuthResultToCookie( auth_result_p, state_machine_p);

    /* Bypass to next auth CSC
     */
    SYS_CALLBACK_MGR_AuthenticatePacket_AsyncReturn(SYS_MODULE_NETACCESS,
                                                    auth_result_p->result,
                                                    state_machine_p->port_security_sm.cookie);
    state_machine_p->port_security_sm.cookie = NULL;

    if(FALSE == NETACCESS_OM_UpdateCookie(lport, state_machine_p->port_security_sm.cookie))
    {
        NETACCESS_VM_DEBUG_PRINT_ERROR("[%s](%lu) Update state machine cookie failed", __FUNCTION__,lport);
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalUpdateAuthResultToCookie
 * ---------------------------------------------------------------------
 * PURPOSE: To update authentication result to cookie.
 * INPUT  : auth_result_p -- authentication result data.
 * OUTPUT : updated_sm_p -- modified cookie data .
 * RETURN : None.
 * NOTES :
 * ---------------------------------------------------------------------
 */
static void NETACCESS_VM_LocalUpdateAuthResultToCookie(
    NETACCESS_VM_AuthResult_T *auth_result_p,
    NETACCESS_OM_StateMachine_T *updated_sm_p)
{
   SYS_CALLBACK_MGR_AuthenticatePacket_CBData_T *cb_data_p = NULL;
   SYS_CALLBACK_MGR_LanReceivePacket_CBData_T   *lan_cbdata_msg_p = NULL;
   cb_data_p = (SYS_CALLBACK_MGR_AuthenticatePacket_CBData_T *)updated_sm_p->port_security_sm.cookie;
   lan_cbdata_msg_p = &cb_data_p->lan_cbdata;

    if ((0 != auth_result_p->vid)
        &&(lan_cbdata_msg_p->tag_info != auth_result_p->vid)
      )
   {
        lan_cbdata_msg_p->tag_info = auth_result_p->vid;
   }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_LocalEnterDot1xAuthFailedState
 * ---------------------------------------------------------------------
 * PURPOSE: To process Dot1X auth failed state
 * INPUT  : lport -- port number
  *         state_machine_p -- status of running state
 * OUTPUT : None.
 * RETURN : TRUE -- process succeed
 *          FALSE -- process failed.
 * NOTES :
 * ---------------------------------------------------------------------
 */
static BOOL_T NETACCESS_VM_LocalEnterDot1xAuthFailedState(
    UI32_T lport,
    NETACCESS_OM_StateMachine_T *state_machine_p)
{
#if (SYS_CPNT_DOT1X == TRUE)
    BOOL_T      have_dot1x_mac;

    if (1 == state_machine_p->port_security_sm.event_bitmap.dot1x_logon)
    {
        /* reauthentication failed
         * or received EAP-LOGOFF
         */
        if(   (1 == state_machine_p->port_security_sm.event_bitmap.waiting_reauth_result)
           || (1 == state_machine_p->port_security_sm.event_bitmap.dot1x_logoff)
          )
        {
            if (DOT1X_OM_IsPortBasedMode(lport))
            {
                if(FALSE == NETACCESS_VM_DeleteAllAuthorizedUserByPort(lport))
                {
                    NETACCESS_VM_DEBUG_PRINT_ERROR(
                        "NETACCESS_VM_DeleteAllAuthorizedUserByPort(%lu) failed", lport);
                    NETACCESS_VM_NotifyDot1xManagerSetupResult(lport, state_machine_p, FALSE);
                    return FALSE;
                }
                state_machine_p->port_security_sm.event_bitmap.dot1x_logon = 0;
            }
            else
            {
                /* remove the authenticating MAC when dot1x run in mac-based
                 */
                if (FALSE == NETACCESS_VM_LocalDeleteAuthenticatingMac(lport, state_machine_p))
                {
                    NETACCESS_VM_DEBUG_PRINT_ERROR(
                                   "NETACCESS_VM_LocalDeleteAuthenticatingMac(%lu) failed", lport);
                    NETACCESS_VM_NotifyDot1xManagerSetupResult(lport, state_machine_p, FALSE);
                    return FALSE;
                }

                have_dot1x_mac = NETACCESS_VM_LocalHaveDot1xMac(lport);

                /* if there have no any dot1x authorized MAC on the port,
                 * restore auto VLAN configuration and clear dot1x_logon flag
                 */
                if (FALSE == have_dot1x_mac)
                {
                    NETACCESS_VM_ResetVlanQos2Default(lport);
                    state_machine_p->port_security_sm.event_bitmap.dot1x_logon = 0;
                }
            }
        }
    }

    /* join to th-fail VLAN if it be configured
     */
    if (   (1 == state_machine_p->port_security_sm.event_bitmap.dot1x_fail)
        && (FALSE == NETACCESS_VM_IsInRestrictedVlan(lport))
        && (TRUE == NETACCESS_VM_LocalIsNeedToJoinAuthFailVlan(lport))
       )
    {
        if (    (FALSE == NETACCESS_VM_SetToGuestVlan(lport))
            ||  (FALSE == NETACCESS_VM_LocalEnableAutoLearning(lport))
           )
        {
            NETACCESS_VM_NotifyDot1xManagerSetupResult(lport, state_machine_p, FALSE);
            return FALSE;
        }
    }

    NETACCESS_VM_NotifyDot1xManagerSetupResult(lport, state_machine_p, TRUE);
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_VM_IsMacAuthEnabled
 *-------------------------------------------------------------------------
 * PURPOSE : Check mac-authentication is enabled on port.
 * INPUT   : lport -- checked port number
 * OUTPUT  : None.
 * RETURN  : TRUE  -- enabled
 *           FALSE -- disabled
 * NOTE    :
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_VM_IsMacAuthEnabled(UI32_T lport)
{
    NETACCESS_PortMode_T port_mode;

    if ( (TRUE == NETACCESS_OM_GetSecurePortMode(lport, &port_mode))
        &&( (port_mode == NETACCESS_PORTMODE_MAC_AUTHENTICATION)
         || (port_mode == NETACCESS_PORTMODE_MAC_ADDRESS_OR_USER_LOGIN_SECURE)
         || (port_mode == NETACCESS_PORTMODE_MAC_ADDRESS_ELSE_USER_LOGIN_SECURE))
       )
    {
        return TRUE;
    }

    return FALSE;
}

#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
