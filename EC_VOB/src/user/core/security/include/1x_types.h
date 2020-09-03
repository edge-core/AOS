/* Project Name : New Feature
 * File_Name    : 1x_types.h
 * Purpose      : IEEE 802.1x Implementation
 *                Contains all declarations of all common types.
 * 2002/05/07   : Kevin Cheng  Create this file
 *
 * Copyright(C) Accton Corporation, 2001, 2002
 *
 * Note         : Designed for new platform (Mercury)
 */
#ifndef LIB1x_TYPES_H
#define LIB1x_TYPES_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "l_mm.h"
#include "leaf_ieee8021x.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define	FALSE 	0
#define TRUE	1

/**** Add for MIB ****/
/** Session Terminate Cause **/
#define SUPPLICANT_LOGOFF           1
#define PORT_FAILURE                2
#define SUPPLICANT_RESTART          3
#define REAUTH_FAILED               4
#define AUTH_CONTROL_FORCE_UNAUTH   5
#define PORT_REINIT                 6
#define PORT_ADMIN_DISABLED         7
#define NOT_TERMINATED_YET        999

/** Session Authentic Method **/
#define REMOTE_AUTH_SERVER          1
#define LOCAL_AUTH_SERVER           2

#define	LIB1X_EAP_REQUEST	1
#define LIB1X_EAP_RESPONSE	2
#define LIB1X_EAP_SUCCESS	3
#define LIB1X_EAP_FAILURE	4

#define DOT1X_SESSION_ID_LENGTH     25
#define DOT1X_USERNAME_LENGTH       32

enum
{
    DOT1X_TYPE_AUTH_RESULT_SUCCESS,
    DOT1X_TYPE_AUTH_RESULT_FAIL,
    DOT1X_TYPE_AUTH_RESULT_LOGOFF, /* 802.1X client send eap-logoff (no need to do authentication) */
    DOT1X_TYPE_AUTH_RESULT_NO_EAPOL
};

/* MACRO FUNCTION DECLARATIONS */

/* DATA TYPE DECLARATIONS
 */
typedef struct DOT1X_PaePortEntry_S
{
    UI32_T dot1xPaePortNumber;
    UI32_T dot1xPaePortProtocolVersion;
    UI32_T dot1xPaePortCapabilities;
    BOOL_T dot1xPaePortInitialize;
    BOOL_T dot1xPaePortReauthenticate;
} DOT1X_PaePortEntry_T;

typedef struct DOT1X_AuthConfigEntry_S
{
    UI32_T dot1xAuthPaeState;
    UI32_T dot1xAuthBackendAuthState;
    UI32_T dot1xAuthAdminControlledDirections;
    UI32_T dot1xAuthOperControlledDirections;
    UI32_T dot1xAuthAuthControlledPortStatus;
    UI32_T dot1xAuthAuthControlledPortControl;
    UI32_T dot1xAuthQuietPeriod;
    UI32_T dot1xAuthTxPeriod;
    UI32_T dot1xAuthSuppTimeout;
    UI32_T dot1xAuthServerTimeout;
    UI32_T dot1xAuthMaxReq;
    UI32_T dot1xAuthReAuthPeriod;
    BOOL_T dot1xAuthReAuthEnabled;
    BOOL_T dot1xAuthKeyTxEnabled;
} DOT1X_AuthConfigEntry_T;

typedef struct DOT1X_AuthStatsEntry_S
{
    UI32_T dot1xAuthEapolFramesRx;
    UI32_T dot1xAuthEapolFramesTx;
    UI32_T dot1xAuthEapolStartFramesRx;
    UI32_T dot1xAuthEapolLogoffFramesRx;
    UI32_T dot1xAuthEapolRespIdFramesRx;
    UI32_T dot1xAuthEapolRepsFramesRx;
    UI32_T dot1xAuthEapolReqIdFramesTx;
    UI32_T dot1xAuthEapolReqFramesTx;
    UI32_T dot1xAuthInvalidEapolFramesRx;
    UI32_T dot1xAuthEapLengthErrorFramesRx;
    UI32_T dot1xAuthLastEapolFrameVersion;
    UI8_T dot1xAuthLastEapolFramesSource[6];
} DOT1X_AuthStatsEntry_T;

typedef struct DOT1X_AuthDiagEntry_S
{
    UI32_T dot1xAuthEntersConnecting;
    UI32_T dot1xAuthEapLogoffsWhileConnecting;
    UI32_T dot1xAuthEntersAuthenticating;
    UI32_T dot1xAuthAuthSuccessWhileAuthenticating;
    UI32_T dot1xAuthAuthTimeoutsWhileAuthenticating;
    UI32_T dot1xAuthAuthFailWhileAuthenticating;
    UI32_T dot1xAuthAuthReauthsWhileAuthenticating;
    UI32_T dot1xAuthAuthEapStartsWhileAuthenticating;
    UI32_T dot1xAuthAuthEapLogoffWhileAuthenticating;
    UI32_T dot1xAuthAuthReauthsWhileAuthenticated;
    UI32_T dot1xAuthAuthEapStartsWhileAuthenticated;
    UI32_T dot1xAuthAuthEapLogoffWhileAuthenticated;
    UI32_T dot1xAuthBackendResponses;
    UI32_T dot1xAuthBackendAccessChallenges;
    UI32_T dot1xAuthBackendOtherRequestsToSupplicant;
    UI32_T dot1xAuthBackendNonNakResponsesFromSupplicant;
    UI32_T dot1xAuthBackendAuthSuccesses;
    UI32_T dot1xAuthBackendAuthFails;
} DOT1X_AuthDiagEntry_T;

typedef struct DOT1X_AuthSessionStatsEntry_S
{
    UI64_T dot1xAuthSessionOctetsRx;
    UI64_T dot1xAuthSessionOctetsTx;
    UI32_T dot1xAuthSessionFramesRx;
    UI32_T dot1xAuthSessionFramesTx;
    char   dot1xAuthSessionId[DOT1X_SESSION_ID_LENGTH + 1];
    UI32_T dot1xAuthSessionAuthenticMethod;
    UI32_T dot1xAuthSessionTime;
    UI32_T dot1xAuthSessionTerminateCause;
    BOOL_T dot1xAuthSessionEverSuccess;
    char   dot1xAuthSessionUserName[DOT1X_USERNAME_LENGTH + 1];
} DOT1X_AuthSessionStatsEntry_T;

typedef struct DOT1X_Global_Parameters_S
{
    UI32_T reauth_enabled;
    UI32_T reauth_period;
    UI32_T quiet_period;
    UI32_T tx_period;
    UI32_T supp_timeout;
    UI32_T server_timeout;
    UI32_T reauth_max;
    UI32_T max_req;
} DOT1X_Global_Parameters_T;

typedef struct DOT1X_PortDetails_S
{
    UI32_T status;
    UI32_T port_control;
    UI8_T supplicant[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T current_id;
    UI32_T apsm_state;
    UI32_T reauth_count;
    UI32_T basm_state;
    UI32_T request_count;
    UI32_T identifier;
    UI32_T resm_state;
} DOT1X_PortDetails_T;

/* use SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_DOT1X_AUTH_RESULT
 *   to send back the result on linux platform.
 */
#if 0
/* cookie, announce authorized result function pointer */
typedef BOOL_T (*DOT1X_AuthorizedResultCookie_T)(
                UI32_T lport,
                UI8_T *mac,
                int eap_identifier,
                UI8_T authorized_result, /* DOT1X_TYPE_AUTH_RESULT_SUCCESS/FAIL/LOGOFF */
                UI8_T *authorized_vlan_list,
                UI8_T *authorized_qos_list,
                UI32_T session_time,
                UI32_T server_ip);
#endif

typedef struct
{
    UI16_T      msg_type;   /* 4 bytes              */
    UI8_T       saddr[6];   /* 6 bytes              */
    L_MM_Mref_Handle_T    *mem_ref;   /* 4 bytes              */
    UI16_T      unit_no;    /* 2 bytes              */
    UI16_T      port_no;     /* 2 bytes              */
} DOT1X_MSG_T;               /* 16 bytes in total    */


typedef struct
{
    UI16_T      msg_type;   /* 4 bytes              */
    UI8_T       *data;      /* 6 bytes              */
    UI32_T      result;
} DOT1X_RADIUS_T;               /* 16 bytes in total    */

typedef struct
{
    UI32_T      unit;
    UI32_T      port;
} DOT1X_PORT_T;

typedef struct  DOT1X_TASK_MSGQ_S
{
    UI8_T 	dot1x_task_service_type;      /* encoding message type, source    */
    UI8_T   *mtext;      /* point message block  */
    UI16_T	mreserved1;
    UI32_T 	mreserved2;  /* not defined for future extension */
    UI32_T 	mreserved3;  /* not defined for future extension */
    UI32_T 	mreserved4;  /* not defined for future extension */
}   DOT1X_TASK_MSGQ_T;

typedef	struct DOT1X_TASK_MSG_S
{
	//L_MREF_T	*mem_ref;
 	UI8_T		dst_mac[6];
	UI8_T		src_mac[6];
	UI16_T		tag_info ;
 	UI16_T		type;
    UI32_T      pkt_length;
    UI32_T      unit_no;
    UI32_T      port_no;
	UI32_T      lport_no;
	UI32_T      vid;
}DOT1X_TASK_MSG_T;

typedef     struct RADIUS_TASK_MSG_S
{
	UI8_T       src_mac[6];
	UI32_T      lport_no;
	UI32_T      vid;
}RADIUS_TASK_MSG_T;

typedef	struct DOT1X_ASYNC_AUTH_MSG_S
{
	UI32_T  lport;
	UI8_T	src_mac[6];
	UI8_T   dst_mac[6];
	UI16_T	tag_info;
	UI16_T	type;
	UI8_T   *eappkt_data;
    UI32_T	eappkt_length;
	UI32_T  cookie; /* MSGQ_ID to send message on linux platform */
	UI32_T  vid;
}DOT1X_ASYNC_AUTH_MSG_T;

enum DOT1X_EVENT_MASK_E
{
    DOT1X_EVENT_NONE       =   0x0000L,
    DOT1X_EVENT_RECVPKT    =   0x0001L,
    DOT1X_EVENT_TIMER      =   0x0002L,
    DOT1X_EVENT_RADIUSPKT  =   0x0004L,
    DOT1X_EVENT_PORTDOWN  =   0x0008L,
    DOT1X_EVENT_PORTUP       =   0x0010L,
    DOT1X_EVENT_REAUTH       =   0x0020L,
    DOT1X_EVENT_ENTER_TRANSITION = 0x0040L,
    DOT1X_EVENT_ASYNCAUTHCHECK		= 0x0080L,
    DOT1X_EVENT_ALL        =   0xFFFFL
};

enum DOT1X_MSG_TYPE_E
{
    DOT1X_MSG_DOT1XPDU        =   1,
    DOT1X_MSG_REQUEST,
    DOT1X_MSG_SERVICE,
    DOT1X_MSG_MISC,
    DOT1X_MSG_RADIUSPDU
};

enum DOT1XMAC_TIMER_TYPE_E
{
    /*DOT1XMAC_TIMER_TYPE_AWHILE        =   1,*/
    DOT1XMAC_TIMER_TYPE_QUIETWHILE	=   1,
    DOT1XMAC_TIMER_TYPE_TXWHILE,
    DOT1XMAC_TIMER_TYPE_SUPPLICANTWHILE,
    DOT1XMAC_TIMER_TYPE_SERVERWHILE,
    DOT1XMAC_TIMER_TYPE_REAUTHWHILE
};

enum DOT1XMAC_TIMER_EVENT_E
{
    /*DOT1XMAC_TIMER_EVENT_AWHILE          =   0x0001L,*/
    DOT1XMAC_TIMER_EVENT_QUIETWHILE        =   0x0001L,
    DOT1XMAC_TIMER_EVENT_TXWHILE           =   0x0002L,
    DOT1XMAC_TIMER_EVENT_SUPPLICANTWHILE   =   0x0004L,
    DOT1XMAC_TIMER_EVENT_SERVERWHILE       =   0x0008L,
    DOT1XMAC_TIMER_EVENT_REAUTHWHILE       =   0x0010L
};
enum DOT1X_RADA_MODE_TYPE_E
{
    DOT1X_RADA_MODE_NORESTRICTION       =   1,
    DOT1X_RADA_MODE_CONTINUOUSLEARNING,
    DOT1X_RADA_MODE_AUTOLEARN,
    DOT1X_RADA_MODE_SECURE,
    DOT1X_RADA_MODE_USERLOGIN,
    DOT1X_RADA_MODE_USERLOGINSECURE,
    DOT1X_RADA_MODE_RADA,
    DOT1X_RADA_MODE_RADA_OR_USERLOGINSECURE,
    DOT1X_RADA_MODE_RADA_ELSE_USERLOGINSECURE,
    DOT1X_RADA_MODE_MACBASED_1X,
};

enum DOT1X_TASK_SERVICE_TYPE_E
{
	DOT1X_TASK_ASYNC_AUTH_CHECK       =   0,
	DOT1X_TASK_MAX_SERVICE_NO
};

typedef enum
{
    DOT1X_TYPE_AUTH_CONTROLLED_PORT_STATUS_AUTHORIZED = VAL_dot1xAuthAuthControlledPortStatus_authorized,
    DOT1X_TYPE_AUTH_CONTROLLED_PORT_STATUS_UNAUTHORIZED = VAL_dot1xAuthAuthControlledPortStatus_unauthorized,
    DOT1X_TYPE_AUTH_CONTROLLED_PORT_STATUS_ERR,
}DOT1X_TYPE_AuthControlledPortStatus_T;

/* for trace_id of user_id when allocate buffer with l_mm
 */
enum
{
    DOT1X_TYPE_TRACE_ID_DOT1X_MGR_ASYNCAUTHCHECK = 0,
    DOT1X_TYPE_TRACE_ID_DOT1X_TASK_ANNOUNCEDOT1XPACKET,
    DOT1X_TYPE_TRACE_ID_DOT1X_TASK_ANNOUNCEINTRUSIONMAC,
    DOT1X_TYPE_TRACE_ID_DOT1X_SEND_PACKET
};

typedef enum	{ apsm_Initialize, apsm_Disconnected, apsm_Waiting, apsm_Connecting, apsm_Authenticating, apsm_Authenticated, apsm_Aborting,
		apsm_Held, apsm_Force_Auth, apsm_Force_Unauth, apsm_Response, apsm_Request, apsm_Timeout, apsm_Reconnect, apsm_Close }	AUTH_PAE_STATE;

typedef enum    { kxsm_No_Key_Transmit, kxsm_Key_Transmit }	AUTH_KEYSM;

typedef enum	{ pmt_ForceUnauthorized, pmt_ForceAuthorized, pmt_Auto }	PORT_MODE_TYPE;
/*typedef enum	{ pst_Unauthorized, pst_Authorized }	PORT_STATUS_TYPE;*/


typedef enum 	{ basm_Request, basm_Response, basm_Success, basm_Fail, basm_Timeout, basm_Idle, basm_Initialize } 	BAUTH_SM_STATE;

typedef	enum	{ cdsm_Force_Both, cdsm_In_Or_Both }	CTRL_SM_STATE;

typedef enum 	{ dir_Both,	dir_In }			DIRECTION;

typedef enum	{ spsm_Logoff, spsm_Disconnected, spsm_Held, spsm_Authenticated, spsm_Connecting, spsm_Acquired, spsm_Authenticating }	SUPP_PAE_STATE;

typedef	enum	{ resm_Initialize, resm_Reauthenticate } REAUTH_SM_STATE;
typedef enum	{ krcsm_No_Key_Receive, krcsm_Key_Receive }	KRC_SM;

typedef	enum 	{ role_Authenticator, role_Supplicant } ROLE;

#ifndef UNIT_TEST
typedef int	BOOLEAN;
#endif /* #ifndef UNIT_TEST */

#endif
