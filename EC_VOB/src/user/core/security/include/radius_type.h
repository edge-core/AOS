/* MODULE NAME: radius_type.h
 * PURPOSE:
 *
 * NOTES: this file offer some definitions used by MGR & OM & radius lib
 *
 * History:
 *
 * Copyright(C)      Accton Corporation, 2004
 */
#ifndef RADIUS_TYPE_H
#define RADIUS_TYPE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_dflt.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "l_mm.h"
#include "aaa_def.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define  RADIUS_MAX_MAC_STRING_LENGTH      17 /*xx-xx-xx-xx-xx-xx*/

#if ( SYS_CPNT_DOT1X_TTLS_TLS_PEAP == TRUE)
    #define	    BUFFER_LEN		            4096//256	/* suger, 05-04-2004, max radius packet length is 4096 */
#else
    #define	    BUFFER_LEN		            256
#endif

#define RADIUS_MAX_SECRET_LENGTH           SYS_ADPT_RADIUS_SECRET_KEY_MAX_LENGTH

#define RADIUS_TYPE_DEFAULT_SERVER_SECRET   ""

/* for CLI (move from radius_om.h) */
#define MAX_SECRET_LENGTH		            SYS_ADPT_RADIUS_SECRET_KEY_MAX_LENGTH /* MUST be multiple of 16 */
#define RADIUS_Default_Server_Port          SYS_DFLT_RADIUS_AUTH_CLIENT_SERVER_PORT_NUMBER
#define RADIUS_Default_Retransmit_Times     SYS_DFLT_RADIUS_AUTH_CLIENT_ACCESS_RETRANSMISSIONS
#define RADIUS_Default_Timeout              SYS_DFLT_RADIUS_AUTH_CLIENT_TIMEOUTS
#define RADIUS_Default_Server_IP            SYS_DFLT_RADIUS_AUTH_SERVER_ADDRESS
#define RADIUS_Default_Server_Secret        ""
#define RADIUS_NOT_IN_MASTER_MODE          -3

//#define RADIUS_MGR_MAX_SERVER               1
#define RADIUS_MAX_RETRANSMIT_TIMES        30
#define RADIUS_MIN_RETRANSMIT_TIMES         1
#define RADIUS_MAX_SERVER_PORT          65535
#define RADIUS_MIN_SERVER_PORT              1L
#define RADIUS_MAX_REQUEST_TIMEOUT      65535L
#define RADIUS_MIN_REQUEST_TIMEOUT          1

#define RADIUS_RETRANSMIT_TIMES_FOR_UNREACHABLE_HOST    0
#define RADIUS_TIMEOUT_FOR_UNREACHABLE_HOST             3       /* seconds */

/* for user auth (move from radius_om.h) */
#define AUTH_ADMINISTRATIVE             15
#define AUTH_LOGIN                      0
#define AUTH_LOGIN_ERROR                (-1)

#define NOT_IN_MASTER_MODE_RC           (-3)
#define BADRESP_RC                      (-2)
#define ERROR_RC                        (-1)
#define OK_RC                           0
#define TIMEOUT_RC                      1
#define CHALLENGE_RC                    2
#define FAIL_RC                         3
#define WAIT_RC                         4

/* for 802.1x (move from radiusclient.h) */

#if ( SYS_CPNT_DOT1X_TTLS_TLS_PEAP == TRUE)
#define AUTH_STRING_LEN         253//128    /* suger, 05-04-2004, max attribute length is 253 */
#define EAP_MESSAGE_SIZE        1600//AUTH_STRING_LEN   /* suger, 05-04-2004, enlarge to 1600 */
#else
#define AUTH_STRING_LEN         128 /* suger, 05-04-2004, max attribute length is 253 */
#define EAP_MESSAGE_SIZE        AUTH_STRING_LEN /* suger, 05-04-2004, enlarge to 1600 */
#endif

#define  STATE_MESSAGE_SIZE     AUTH_STRING_LEN

/* port to linux (move from radiusclient.c)
 */
#define RADIUS_USER_NAME_LEN 128

/* port to linux (move from radiusclient.h)
 */
#define AUTH_VECTOR_LEN		16
#define AUTH_PASS_LEN		SYS_ADPT_RADIUS_SECRET_KEY_MAX_LENGTH /* multiple of 16 */


/* MACRO FUNCTION DECLARATIONS
 */





/* DATA TYPE DECLARATIONS
 */
 
typedef enum RADIUS_ReturnValue_E 
{
    RADIUS_RETURN_FAIL,
    RADIUS_RETURN_SUCCESS
} RADIUS_ReturnValue_T;

typedef enum RADIUS_ResultValue_E 
{
    RADIUS_RESULT_FAIL,
    RADIUS_RESULT_SUCCESS,
    RADIUS_RESULT_TIMEOUT
} RADIUS_ResultValue_T;

typedef enum RADIUS_DebugType_E
{
    RADIUS_DEBUG_TASK                       = 0x00000001,
    RADIUS_DEBUG_MGR                        = 0x00000002,
    RADIUS_DEBUG_OM                         = 0x00000004,
    RADIUS_DEBUG_RADEXAMPLE                 = 0x00000008,
    RADIUS_DEBUG_BUILDREQ                   = 0x00000010,
    RADIUS_DEBUG_SENDSERVER                 = 0x00000020,
    RADIUS_DEBUG_AVPAIR                     = 0x00000040
} RADIUS_DebugType_T;

typedef enum
{
    RADIUS_ASYNC_REQ_FLAG_NULL              = 0x00000000,
    RADIUS_ASYNC_REQ_FLAG_CANCEL_REQUEST    = 0x00000001,
} RADIUS_AsyncRequestControlFlag_T;

typedef enum
{
    RADIUS_UNKNOWN  = 0,        /* Not visit yet. Initial status */
    RADIUS_UNREACHABLE,         /* Visited. Unreachable */
    RADIUS_ALIVE,               /* Visited. Alive */
} RADIUS_ServerStatus_T;

typedef struct radiusAuthtab
{
    UI32_T  radiusAuthServerIndex;
    UI32_T  radiusAuthServerAddress;
    UI32_T  radiusAuthClientServerPortNumber;
    UI32_T  radiusAuthClientRoundTripTime;
    UI32_T  radiusAuthClientAccessRequests;
    UI32_T  radiusAuthClientAccessRetransmissions;
    UI32_T  radiusAuthClientAccessAccepts;
    UI32_T  radiusAuthClientAccessRejects;
    UI32_T  radiusAuthClientAccessChallenges;
    UI32_T  radiusAuthClientMalformedAccessResponses;
    UI32_T  radiusAuthClientBadAuthenticators;
    UI32_T  radiusAuthClientPendingRequests;
    UI32_T  radiusAuthClientTimeouts;
    UI32_T  radiusAuthClientUnknownTypes;
    UI32_T  radiusAuthClientPacketsDropped;
}AuthServerEntry;

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)

typedef enum RADACC_ClientEnable_E
{
    RADACC_DISABLE,
    RADACC_ENABLE,
} RADACC_ClientEnable_T;

typedef enum RADACC_AcctStatusType_E
{
    RADACC_START            = 1,
    RADACC_STOP             = 2,
    RADACC_InterimUpdate    = 3,
} RADACC_AcctStatusType_T;

typedef struct RADACC_UserInfoInterface_S
{
    UI16_T  user_index;     /* array index + 1 */
    UI32_T  ifindex;        /* if client_type == AAA_CLIENT_TYPE_DOT1X, ifindex == l_port
                               if client_type == AAA_CLIENT_TYPE_EXEC, ifindex implies console or telnet's session id
                             */
    UI8_T   user_name[SYS_ADPT_MAX_USER_NAME_LEN + 1];
    UI32_T  accounting_start_time;  /* unit is seconds */

    AAA_ClientType_T         client_type;
} RADACC_UserInfoInterface_T;

typedef struct RADACC_Rfc2620_S
{
    /* mib definitions in RFC2620 */
    UI32_T  radiusAccServerAddress;
    UI32_T  radiusAccClientServerPortNumber;

    UI32_T  radiusAccClientRoundTripTime;
    UI32_T  radiusAccClientRequests;        /* = response + pending request + client timeouts */
    UI32_T  radiusAccClientRetransmissions;
    UI32_T  radiusAccClientResponses;
    UI32_T  radiusAccClientMalformedResponses;
    UI32_T  radiusAccClientBadAuthenticators;
    UI32_T  radiusAccClientPendingRequests;
    UI32_T  radiusAccClientTimeouts;
    UI32_T  radiusAccClientUnknownTypes;
    UI32_T  radiusAccClientPacketsDropped;
} RADACC_Rfc2620_T;

#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

typedef struct RADIUS_Server_Host_S {
    BOOL_T   used_flag;
    UI32_T   server_ip;
    UI32_T   server_port;
    UI32_T   timeout;
    UI32_T   retransmit;
    UI32_T   server_index;
    UI8_T    secret[MAXSIZE_radiusServerGlobalKey+1];
    AuthServerEntry server_table;
    RADIUS_ServerStatus_T   status;

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    UI32_T  acct_port;
    RADACC_Rfc2620_T    std_acc_cli_mib;
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */
} RADIUS_Server_Host_T;

/* for 802.1x (move from radiusclient.h) */
typedef struct recv_eap_data
{
    UI8_T          data[EAP_MESSAGE_SIZE + 1];
    UI32_T         len;
    UI32_T         src_port;
    UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T         src_vid;
    UI32_T         cookie; /* MSGQ_ID for return result */
    UI32_T         server_ip;
} R_EAP_DATA;

typedef struct recv_state_data
{
    UI8_T          data[STATE_MESSAGE_SIZE+1];
    UI32_T         len;
} R_STATE_DATA;

enum
{
    RADIUS_TYPE_TRACE_ID_RC_AVPAIR_NEW = 0,
    RADIUS_TYPE_TRACE_ID_RC_AVPAIR_GEN,
	RADIUS_TYPE_TRACE_ID_RC_VENDOR_AVPAIR_GEN,
    RADIUS_TYPE_TRACE_ID_RADIUS_MGR_RECEIVEPACKETFROMSOCKET,
};

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */


#endif /* End of RADIUS_TYPE_H */
