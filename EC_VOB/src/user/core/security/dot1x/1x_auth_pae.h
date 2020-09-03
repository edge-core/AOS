/* MODULE NAME: 1x_auth_pae.h
 * PURPOSE:
 *  802.1X authenticator PAE state machine
 *
 * NOTES:
 *
 * History:
 *    2002/06/25  : Kevin Cheng     Create this file
 *    2004/09/22  : mfhorng         add lib1x_SetupAmtrAndSwctrl()
 *
 * Copyright(C)      Accton Corporation, 2004
 */

#ifndef LIB1X_AUTH_PAE_H
#define LIB1X_AUTH_PAE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "leaf_ieee8021x.h"
#include "1x_types.h"
#include "1x_common.h"
#include "1x_ethernet.h"
#include "1x_reauth_sm.h"
#include "1x_bauth_sm.h"
#include "1x_cdsm.h"
#include "1x_krc_sm.h"
#include "1x_kxsm.h"
#include "1x_ptsm.h"
#include "1x_radius.h"
#include "sys_dflt.h"
#include <string.h>
#include "swdrv_type.h"
#include "radius_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/*            The default Value           */
#define DOT1X_DEFAULT_QUIETPERIOD         SYS_DFLT_DOT1X_AUTH_QUIET_PERIOD
#define DOT1X_DEFAULT_TXPERIOD            SYS_DFLT_DOT1X_AUTH_TX_PERIOD
#define DOT1X_DEFAULT_SUPPLICANT_TIMEOUT  SYS_DFLT_DOT1X_AUTH_SUPP_TIMEOUT
#define DOT1X_DEFAULT_SERVER_TIMEOUT      SYS_DFLT_DOT1X_AUTH_SERVER_TIMEOUT
#define DOT1X_DEFAULT_MAXREQ              SYS_DFLT_DOT1X_AUTH_MAX_REQ
#define DOT1X_DEFAULT_REAUTHPERIOD        SYS_DFLT_DOT1X_AUTH_REAUTH_PERIOD

#define DOT1X_DEFAULT_PORT_MODE           SYS_DFLT_DOT1X_AUTH_CONTROLLED_PORT_CONTROL

#define DOT1X_DEFAULT_REAUTHENABLED       SYS_DFLT_DOT1X_AUTH_REAUTH_ENABLED

#define DOT1X_DEFAULT_MULTIPLEHOST        FALSE
#define DOT1X_DEFAULT_REAUTHMAX           2
#define DOT1X_DEFAULT_PORT_RADA_MODE	DOT1X_RADA_MODE_NONE
/*system operation mode*/
#define DOT1X_PORT_OPERATION_MODE_ONEPASS	1
#define DOT1X_PORT_OPERATION_MODE_MULTIPASS	2
#define DOT1X_PORT_OPERATION_MODE_MACBASED      3
#define DOT1X_DEFAULT_PORT_OPERATION_MODE	DOT1X_PORT_OPERATION_MODE_ONEPASS
/* max MAC count*/
#define DOT1X_DEFAULT_MULTI_HOST_MAC_COUNT	5

#define DOT1X_VM_Do_Authenticator             lib1x_do_authenticator

#define LIB1X_AP_QUIET_PERIOD		 	DOT1X_DEFAULT_QUIETPERIOD  	/* seconds*/
#define LIB1X_AP_REAUTHMAX			DOT1X_DEFAULT_REAUTHMAX	/* attempts*/
#define LIB1X_AP_TXPERIOD			DOT1X_DEFAULT_TXPERIOD	/* seconds*/

#if ( SYS_CPNT_DOT1X_TTLS_TLS_PEAP == TRUE)
    #define LIB1X_AP_SENDBUFLEN		        1600//512 	/* 65535*/	/* suger, 05-04-2004, enlarge to 1600 */
#else
    #define LIB1X_AP_SENDBUFLEN		        512
#endif

#define MAX_EAP_BUFFER				10

#define DOT1X_PTSM_NONE		                0
#define DOT1X_FROM_SUPPLICANT                   1
#define DOT1X_FROM_SERVER                       2
#define DOT1X_PORT_LINKDOWN                     3
#define DOT1X_PORT_LINKUP                       4
#define DOT1X_PTSM_TIMEOUT                      5
#define DOT1X_DO_INITIALIZE                     6
#define DOT1X_DO_REAUTHENTICATE                 7
#define DOT1X_PORT_CONTROL                      8
#define DOT1X_PORT_ADMINUP                      9
#define DOT1X_PORT_ADMINDOWN                   10
#define DOT1X_NETACCESS_CONTROL                11

/* ES4649-32-01078
   obsoleted by DOT1X_USERNAME_LENGTH in 1x_types.h
#define DOT1X_VM_USERNAME_LENGTH                   32
*/

#define DOT1X_MAX_PORT                          SYS_ADPT_TOTAL_NBR_OF_LPORT


#if ( SYS_CPNT_DOT1X_TTLS_TLS_PEAP == TRUE)
    #define DOT1X_PACKET_BUFFER_LEN                 1600//512	/* suger, 05-04-2004, enlarge to 1600 */
#else
#define DOT1X_PACKET_BUFFER_LEN                 512
#endif

#define DOT1X_DISCARD                            0
#define DOT1X_TRANSPARENT                        1
#define DOT1X_AUTHENTICATOR                      2
#if 0
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
#endif
/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/*Global_Params  *gp[DOT1X_MAX_PORT+1];*/

typedef struct DOT1X_PACKET_RX_S
{
    UI8_T    *packet_data;
    UI8_T    dst_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T    src_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI16_T   tag_info;
    UI16_T   packet_type;
    UI32_T   packet_length;
    UI32_T   src_lport;
    UI32_T   src_vid;

} DOT1X_PACKET_T;

typedef struct RADIUS_PACKET_RX_S
{
    UI8_T   packet_data[BUFFER_LEN];
    UI32_T  packet_length;
    UI32_T  radius_auth_result;
    UI32_T  authenticated_vid;  /* no used ? */
    char    *authorized_vlan_list;
    char    *authorized_qos_list;
    UI32_T   authorized_session_time;
    UI32_T   server_ip;
} RADIUS_PACKET_T;

struct pktbuf
{
	UI8_T    pkt[DOT1X_PACKET_BUFFER_LEN];
	UI32_T   length;	/* -1 indicates no packet is stored in this slot*/

#if 0 /* (SAVE MEMORY) the following data members are never used */
	UI8_T    eap_code;
	UI8_T    eap_id;
	UI8_T    eaprr_type;   /* = 255 if not valid i.e. the EAP packet is not a request/response pkt*/
#endif
};

/**********For MIB ***************/
#if 0
typedef enum DOT1X_PAE_CAPABILITIES
{
    DOT1X_CAPABILITIES_AUTHENTICATOR = 0 ,
    DOT1X_CAPABILITIES_SUPPLICANT,
    DOT1X_CAPABILITIES_BOTH,
    DOT1X_CAPABILITIES_NEITHER,
}PAE_CAPABILITIES_TYPE;
#endif
typedef struct VM_Dot1xPaePortTable
{
 UI32_T dot1xPaePortNumber;
 UI32_T dot1xPaePortProtocolVersion;
 UI32_T  dot1xPaePortCapabilities;
 BOOL_T dot1xPaePortInitialize;
 BOOL_T dot1xPaePortReauthenticate;
}VM_Dot1xPaePortEntry;

typedef struct VM_Dot1xAuthConfigTable
{
 int dot1xAuthPaeState;
 int dot1xAuthBackendAuthState;
 DIRECTION dot1xAuthAdminControlledDirections;
 DIRECTION dot1xAuthOperControlledDirections;
 /*PORT_STATUS_TYPE*/UI32_T dot1xAuthAuthControlledPortStatus;
 PORT_MODE_TYPE dot1xAuthAuthControlledPortControl;
 UI32_T dot1xAuthQuietPeriod;
 UI32_T dot1xAuthTxPeriod;
 UI32_T dot1xAuthSuppTimeout;
 UI32_T dot1xAuthServerTimeout;
 UI32_T dot1xAuthMaxReq;
 UI32_T dot1xAuthReAuthPeriod;
 BOOL_T dot1xAuthReAuthEnabled;
 BOOL_T dot1xAuthKeyTxEnabled;
}VM_Dot1xAuthConfigEntry;

typedef struct VM_Dot1xAuthStateTable
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
 UI8_T  dot1xAuthLastEapolFramesSource[6];
}VM_Dot1xAuthStateEntry;

typedef struct VM_Dot1xAuthDiagTable
{
 UI32_T    dot1xAuthEntersConnecting;
 UI32_T    dot1xAuthEapLogoffsWhileConnecting;
 UI32_T    dot1xAuthEntersAuthenticating;
 UI32_T    dot1xAuthAuthSuccessWhileAuthenticating;
 UI32_T    dot1xAuthAuthTimeoutsWhileAuthenticating;
 UI32_T    dot1xAuthAuthFailWhileAuthenticating;
 UI32_T    dot1xAuthAuthReauthsWhileAuthenticating;
 UI32_T    dot1xAuthAuthEapStartsWhileAuthenticating;
 UI32_T    dot1xAuthAuthEapLogoffWhileAuthenticating;
 UI32_T    dot1xAuthAuthReauthsWhileAuthenticated;
 UI32_T    dot1xAuthAuthEapStartsWhileAuthenticated;
 UI32_T    dot1xAuthAuthEapLogoffWhileAuthenticated;
 UI32_T    dot1xAuthBackendResponses;
 UI32_T    dot1xAuthBackendAccessChallenges;
 UI32_T    dot1xAuthBackendOtherRequestsToSupplicant;
 UI32_T    dot1xAuthBackendNonNakResponsesFromSupplicant;
 UI32_T    dot1xAuthBackendAuthSuccesses;
 UI32_T    dot1xAuthBackendAuthFails;
}VM_Dot1xAuthDiagEntry;

#define DOT1X_VM_SESSION_ID_LENGTH     25
#define DOT1X_USERNAME_LENGTH                   32
typedef struct VM_Dot1xAuthSessionStatsTable
{
 UI64_T    dot1xAuthSessionOctetsRx;
 UI64_T    dot1xAuthSessionOctetsTx;
 UI32_T    dot1xAuthSessionFramesRx;
 UI32_T    dot1xAuthSessionFramesTx;
 UI8_T     dot1xAuthSessionId[DOT1X_VM_SESSION_ID_LENGTH+1];
 UI32_T    dot1xAuthSessionAuthenticMethod;
 UI32_T    dot1xAuthSessionTime;
 UI32_T    dot1xAuthSessionTerminateCause;
 UI8_T     dot1xAuthSessionUserName[DOT1X_USERNAME_LENGTH + 1];
}VM_Dot1xAuthSessionStatsEntry;

/************************************************/
struct Auth_Pae_tag
{
	/* The machine state*/
	AUTH_PAE_STATE		state;

	/* The Variables.*/
	BOOLEAN		   	  eapLogoff;
	BOOLEAN			   eapStart;
	PORT_MODE_TYPE		portMode;
	int			       reAuthCount;
	BOOLEAN			   rxRespId;

	BOOLEAN			isSuppPresent;	/* true if we have a supp to communicate with*/

        BOOLEAN                 multiple_hosts;
	/* The constants*/
	int			reAuthMax;


	/* A listing of the other state machines we are going to use.
	   TODO something about the port timers machine.*/
	/* Reauthentication Timer State Machine*/
        Reauth_SM 	reauth_sm;
	/* Backend Authentication State Machine*/
        Bauth_SM	bauth_sm;
	/* Controlled Directions State Machine*/
        CtrlDirSM 	ctrl_sm;
	/* Key Receive State Machine*/
        Krc_SM		krc_sm;
	/* Authenticator Key Transmit State Machine*/
        Auth_KeyxmitSM	keyxmit_sm;


    UI8_T		 sendBuffer[DOT1X_PACKET_BUFFER_LEN+1];
    int		   sendbuflen;
    BOOLEAN            sendreplyready;		/* signals that the reply is ready */
    struct lib1x_eap * send_eapptr;		/* used to communicate the start of eap pkt in sendbuf */
    /*UI8_T		supp_addr[ETHER_ADDRLEN];*/
    /*UI8_T		svr_addr[ETHER_ADDRLEN];*/
    struct Global_Params_tag	* global;
    struct pktbuf   fromsvr; 		/* supplicant / server*/
    struct lib1x_nal_intfdesc  network_svr;
    struct lib1x_nal_intfdesc  network_supp;
   /*UI32_T 	udp_ourport, udp_svrport;*/
    struct radius_info    rinfo;
    /*add by Kevin*/
    DOT1X_PACKET_T dot1x_packet_t;
    RADIUS_PACKET_T radius_packet_t;
    VM_Dot1xAuthStateEntry AuthStateEntry;
    VM_Dot1xAuthDiagEntry AuthDiagEntry;
    VM_Dot1xAuthSessionStatsEntry AuthSessionStatsEntry;
    SWDRV_IfTableStats_T if_table_stats;

    UI32_T action_status;           /* intrusion action status */

    BOOL_T is_auth_session_success;  /* if TRUE,can set termination cause */
    BOOL_T is_ever_pass_auth; /* if TRUE,termination cause have meaning,or should not treat meaningful */
};
typedef struct Auth_Pae_tag Auth_Pae;
struct Auth_Pae_tag       auth_p[DOT1X_MAX_PORT+1];

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* Now the function declarations follow: */
void lib1x_do_authenticator( Global_Params * global,UI32_T which );
Global_Params *  lib1x_init_authenticator( /* UI8_T * svr_addr ,*//*  UI8_T * ourip_addr,*//* UI8_T * svrip_addr,*/
		/*UI32_T udp_ourport,*//* UI32_T udp_svrport , */
		/*UI8_T * dev_svr,*//* UI8_T * dev_supp,*/UI32_T port_number);
/* packet xmit routines.*/
void lib1x_auth_txCannedSuccess( Auth_Pae * auth_pae, int identifier );
void lib1x_auth_txCannedFail( Auth_Pae * auth_pae, int identifier, BOOL_T is_logoff );
void lib1x_auth_txReqId( Auth_Pae * auth_pae, int identifier );
void lib1x_authsm_listen_action( Global_Params * global ,UI32_T which);
Global_Params *  DOT1XMAC_Init_Authenticator( UI8_T * svr_addr , UI8_T * ourip_addr, UI8_T * svrip_addr,
UI32_T udp_ourport, UI32_T udp_svrport , UI8_T * dev_svr, UI8_T * dev_supp,UI32_T mac_entry_index);
void DOT1XMAC_VM_Do_Authenticator( Global_Params * global ,UI32_T which);

BOOL_T lib1x_add_portmac_to_psec(UI32_T ifindex,UI8_T *port_mac,UI32_T vid);
BOOL_T lib1x_remove_portmac_from_psec(UI32_T vid/*ifindex*/,UI8_T *port_mac);
BOOLEAN  lib1x_init_port(Global_Params *global);

/*--------------------------------------------------
 * lib1x_announce_authorized_result:
 *  Send the authentication result to the caller who wanted the 802.1X authentication service.
 *--------------------------------------------------*/
BOOL_T lib1x_announce_authorized_result(UI32_T lport, UI8_T *port_mac, int eap_identifier, UI8_T auth_result,UI8_T *authorized_vlan_list,
															UI8_T *authorized_qos_list,UI32_T session_time,UI32_T server_ip);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - lib1x_SetupAmtrAndSwctrl
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set port auto learning mac counts and auto-learning status
 * INPUT   : ifindex        -- which port to
 *           mac_count      -- mac learning count
 *           enable_auto_learning -- enable auto-learning or not
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T lib1x_SetupAmtrAndSwctrl(UI32_T ifindex, UI32_T mac_count, BOOL_T enable_auto_learning);

#endif  /*LIB1X_AUTH_PAE_H*/
