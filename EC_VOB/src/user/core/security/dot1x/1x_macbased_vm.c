#include "sys_type.h"
#include "1x_common.h"
#include "1x_auth_pae.h"
#include "1x_eapol.h"
#include <stdlib.h>
#include "1x_om.h"
#include "vlan_mgr.h"
#include "1x_macbased_ptsm.h"

static void DOT1XMAC_Authsm_Held( Auth_Pae * auth_pae , Global_Params * global);
static void DOT1XMAC_Authsm_Initialize( Auth_Pae * auth_pae , Global_Params * global);
static void DOT1XMAC_Authsm_Disconnected( Auth_Pae * auth_pae , Global_Params * global);
static void DOT1XMAC_Authsm_Connecting( Auth_Pae * auth_pae , Global_Params * global);
static void DOT1XMAC_Authsm_Reconnect( Auth_Pae * auth_pae, Global_Params * global);
static void DOT1XMAC_Authsm_Response( Auth_Pae * auth_pae, Global_Params * global);
static void DOT1XMAC_Authsm_Request( Auth_Pae * auth_pae, Global_Params * global);
static void DOT1XMAC_Authsm_Timeout( Auth_Pae * auth_pae, Global_Params * global);
static void DOT1XMAC_Authsm_Authenticated( Auth_Pae * auth_pae , Global_Params * global);
static void DOT1XMAC_Authsm_Close( Auth_Pae * auth_pae , Global_Params * global);
//void lib1x_authsm_authenticating( Auth_Pae * auth_pae , Global_Params * global);
//void lib1x_authsm_aborting( Auth_Pae * auth_pae , Global_Params * global);
static void DOT1XMAC_Authsm_ForceUnauth( Auth_Pae * auth_pae , Global_Params * global);
static void DOT1XMAC_Authsm_ForceAuth( Auth_Pae * auth_pae , Global_Params * global);
static void DOT1XMAC_Authsm_ExecuteAuthSM( Global_Params * global );
static BOOL_T DOT1XMAC_Trans_AuthSM( Global_Params * global );


Global_Params *  DOT1XMAC_Init_Authenticator( UI8_T * svr_addr , UI8_T * ourip_addr, UI8_T * svrip_addr,
        UI32_T udp_ourport, UI32_T udp_svrport , UI8_T * dev_svr, UI8_T * dev_supp,UI32_T mac_entry_index)
{
      Global_Params     *   global;
    Auth_Pae    *   auth_pae;
    //UI8_T    device[ DOT1XMAC_MAXDEVLEN ];

      global = DOT1X_OM_Get_MACgp(mac_entry_index);
      if(global == 0)
      {
            //SYSFUN_Printf("\n\rDOT1X L_MEM_Allocate error!!");
            //DOT1XMAC_Show_DebugMessage("global database allocate memory error !\n");
            return NULL;
      }
    auth_pae = DOT1X_OM_Get_MACAuthPae(mac_entry_index);//&(mac_auth_p[mac_entry_index]);

    /* 1. Init port timers*/
//  lib1x_ptsm_initialize( global, &(auth_pae->port_timers) );

    /* 2. Init Reauthentication State Machine*/
    //lib1x_reauthsm_init( &(auth_pae->reauth_sm) );
    //auth_pae->reauth_sm.state = resm_Initialize;    /* Not sure if I should start in the initialize state.*/
      //auth_pae->reauth_sm.reAuthEnabled = FALSE;      /* This should be set from outside later.*/
      //auth_pae->reauth_sm.reAuthPeriod = DOT1XMAC_DEFAULT_REAUTHPERIOD;
    /* 3. Init Backend Authentication State Machine*/
    //lib1x_bauthsm_init( &(auth_pae->bauth_sm) );
      //auth_pae->bauth_sm.maxReq = DOT1XMAC_DEFAULT_MAXREQ;
        /* 4. Init Controlled Directions State Machine*/
//  lib1x_cdsm_init(&(auth_pae -> ctrl_sm ));

    /* 5. Key Receive State Machine*/
//  lib1x_krcsm_init( &(auth_pae -> krc_sm));

    /* 6. Authenticator Key Transmit State Machine*/
//  lib1x_kxsm_init( &(auth_pae -> keyxmit_sm) );

    /* 7. Initialize Authenticator Port Access Entity .. state machine also !*/
    auth_pae-> state = apsm_Initialize;
    auth_pae-> eapLogoff = FALSE;
    auth_pae-> eapStart = FALSE;
    auth_pae-> portMode = VAL_dot1xAuthAuthControlledPortControl_auto;
    auth_pae-> reAuthCount = 0;
    auth_pae-> rxRespId = FALSE;

    /* the constants*/
        auth_pae-> multiple_hosts = DOT1X_DEFAULT_MULTIPLEHOST  ;
    //auth_pae-> quietPeriod = LIB1X_AP_QUIET_PERIOD;
    auth_pae-> reAuthMax = LIB1X_AP_REAUTHMAX;
    //auth_pae-> txPeriod = LIB1X_AP_TXPERIOD;
    auth_pae-> sendbuflen = LIB1X_AP_SENDBUFLEN;
    auth_pae -> isSuppPresent = FALSE;

    /* 8. Now gotta intialize the global state info.*/
    global -> authAbort = FALSE;
    global -> authFail  = FALSE;
    global -> authStart = FALSE;
    global -> authTimeout = FALSE;
    global -> authSuccess = FALSE;
    global -> currentId = 0;
//printf("DOT1XMAC_Init_Authenticator:id[%d]\n",global -> currentId);
    global -> initialize = TRUE/*FALSE*/;
//  global -> portControl = pmt_ForceAuthorized;
    global -> portEnabled = FALSE;
    global -> portStatus = VAL_dot1xAuthAuthControlledPortStatus_unauthorized;
    global -> pre_portStatus = VAL_dot1xAuthAuthControlledPortStatus_unauthorized;
    global -> reAuthenticate = FALSE;
    global -> receivedId = -1;  /* Should be in range 0..255 for legal values.*/
    global -> suppStatus = VAL_dot1xAuthAuthControlledPortStatus_unauthorized;

    global -> currentRole = role_Authenticator;     /* This global is an authenticator currently.*/
    //global -> timers = &(auth_pae->port_timers);
    global -> theAuthenticator = auth_pae;
    global -> theSupplicant = NULL;

      /* for server */
    /*strcpy( device, dev_svr );*/
    /*strncpy(auth_pae -> network_svr.device, device, LIB1X_MAXDEVLEN);
      auth_pae -> network_svr.device[LIB1X_MAXDEVLEN] = '\0';*/
    auth_pae -> network_svr.inttype = LIB1X_IT_UDPSOCK;

#if 0 /* (SAVE MEMORY) following data members were already removed from lib1x_nal_intfdesc */
      memcpy( auth_pae -> network_svr.ouraddr,"", ETHER_ADDRLEN);
      auth_pae -> network_svr.promisc_mode = LIB1X_LSTNR_PROMISCMODE;
      auth_pae -> network_svr.snaplen = LIB1X_LSTNR_SNAPLEN;
      auth_pae -> network_svr.read_timeout = LIB1X_LSTNR_RDTIMEOUT;
      auth_pae -> network_svr.packet_handler = NULL ;
#endif

      /*for supplicant*/
      /* strcpy( device, dev_supp );
      strncpy(auth_pae -> network_supp.device, device, LIB1X_MAXDEVLEN);
      auth_pae -> network_supp.device[LIB1X_MAXDEVLEN] = '\0';*/
    auth_pae -> network_supp.inttype = LIB1X_IT_PKTSOCK;

#if 0 /* (SAVE MEMORY) following data members were already removed from lib1x_nal_intfdesc */
      memcpy( auth_pae -> network_supp.ouraddr,"", ETHER_ADDRLEN);
      auth_pae -> network_supp.promisc_mode = LIB1X_LSTNR_PROMISCMODE;
      auth_pae -> network_supp.snaplen = LIB1X_LSTNR_SNAPLEN;
      auth_pae -> network_supp.read_timeout = LIB1X_LSTNR_RDTIMEOUT;
      auth_pae -> network_supp.packet_handler = NULL ;
#endif

    auth_pae-> global = global;     /* we need a back reference*/
    /*memcpy( auth_pae -> svr_addr, svr_addr, ETHER_ADDRLEN );         */

    /* allocate memory for eap buffer*/
#if 0 /* (SAVE MEMORY) 'fromsupp' was alrady removed from 'auth_pae' */
    auth_pae -> fromsupp.length = 0;
#endif

    auth_pae -> fromsvr.length = 0;
    /*auth_pae -> udp_ourport = udp_ourport;*/
    /*auth_pae -> udp_svrport = udp_svrport;*/
    auth_pae -> sendreplyready = FALSE;
    memset(auth_pae -> rinfo.username,'\0',DOT1X_USERNAME_LENGTH);
    auth_pae -> rinfo.rad_stateavailable = FALSE;
    auth_pae -> rinfo.rad_statelength = 0;
      //DOT1X_OM_Set_RADIUSId(0);

      memset(auth_pae -> dot1x_packet_t.src_mac,0,ETHER_ADDRLEN);
        /*For MIB*/
      auth_pae -> AuthStateEntry.dot1xAuthEapolFramesRx = 0;
      auth_pae -> AuthStateEntry.dot1xAuthEapolFramesTx = 0;
      auth_pae -> AuthStateEntry.dot1xAuthEapolStartFramesRx = 0;
      auth_pae -> AuthStateEntry.dot1xAuthEapolLogoffFramesRx = 0;
      auth_pae -> AuthStateEntry.dot1xAuthEapolRespIdFramesRx = 0;
      auth_pae -> AuthStateEntry.dot1xAuthEapolRepsFramesRx = 0;
      auth_pae -> AuthStateEntry.dot1xAuthEapolReqIdFramesTx = 0;
      auth_pae -> AuthStateEntry.dot1xAuthEapolReqFramesTx = 0;
      auth_pae -> AuthStateEntry.dot1xAuthInvalidEapolFramesRx = 0;
      auth_pae -> AuthStateEntry.dot1xAuthEapLengthErrorFramesRx = 0;
      auth_pae -> AuthStateEntry.dot1xAuthLastEapolFrameVersion = 0;
      memset(auth_pae -> AuthStateEntry.dot1xAuthLastEapolFramesSource,0,ETHER_ADDRLEN);

      auth_pae -> AuthDiagEntry.dot1xAuthEntersConnecting = 0;
      auth_pae -> AuthDiagEntry.dot1xAuthEapLogoffsWhileConnecting = 0;
      auth_pae -> AuthDiagEntry.dot1xAuthEntersAuthenticating = 0;
      auth_pae -> AuthDiagEntry.dot1xAuthAuthSuccessWhileAuthenticating = 0;
      auth_pae -> AuthDiagEntry.dot1xAuthAuthTimeoutsWhileAuthenticating = 0;
      auth_pae -> AuthDiagEntry.dot1xAuthAuthFailWhileAuthenticating = 0;
      auth_pae -> AuthDiagEntry.dot1xAuthAuthReauthsWhileAuthenticating = 0;
      auth_pae -> AuthDiagEntry.dot1xAuthAuthEapStartsWhileAuthenticating = 0;
      auth_pae -> AuthDiagEntry.dot1xAuthAuthEapLogoffWhileAuthenticating = 0;
      auth_pae -> AuthDiagEntry.dot1xAuthAuthReauthsWhileAuthenticated = 0;
      auth_pae -> AuthDiagEntry.dot1xAuthAuthEapStartsWhileAuthenticated = 0;
      auth_pae -> AuthDiagEntry.dot1xAuthAuthEapLogoffWhileAuthenticated = 0;
      auth_pae -> AuthDiagEntry.dot1xAuthBackendResponses = 0;
      auth_pae -> AuthDiagEntry.dot1xAuthBackendAccessChallenges = 0;
      auth_pae -> AuthDiagEntry.dot1xAuthBackendOtherRequestsToSupplicant = 0;
      auth_pae -> AuthDiagEntry.dot1xAuthBackendNonNakResponsesFromSupplicant = 0;
      auth_pae -> AuthDiagEntry.dot1xAuthBackendAuthSuccesses = 0;
      auth_pae -> AuthDiagEntry.dot1xAuthBackendAuthFails = 0;

      auth_pae -> AuthSessionStatsEntry.dot1xAuthSessionOctetsRx = 0;
      auth_pae -> AuthSessionStatsEntry.dot1xAuthSessionOctetsTx = 0;
      auth_pae -> AuthSessionStatsEntry.dot1xAuthSessionFramesRx = 0;
      auth_pae -> AuthSessionStatsEntry.dot1xAuthSessionFramesTx = 0;
      memset(auth_pae -> AuthSessionStatsEntry.dot1xAuthSessionId,'\0',DOT1X_VM_SESSION_ID_LENGTH);
      auth_pae -> AuthSessionStatsEntry.dot1xAuthSessionAuthenticMethod = REMOTE_AUTH_SERVER;/*remoteAuthServer*/
      auth_pae -> AuthSessionStatsEntry.dot1xAuthSessionTime = 0;
      auth_pae -> AuthSessionStatsEntry.dot1xAuthSessionTerminateCause = NOT_TERMINATED_YET;/*notTerminatedYet*/
      memset(auth_pae -> AuthSessionStatsEntry.dot1xAuthSessionUserName,'\0',DOT1X_USERNAME_LENGTH);

      global -> mac_entry_index = mac_entry_index;
      global -> session_start_time = 0;
      global -> authenticating_flag = FALSE;

      memset(global -> first_mac,0,ETHER_ADDRLEN);
      global -> is_first_mac_auth = FALSE;
      global->authorized_vid = 0;
//    global -> operation_mode = DOT1X_PORT_OPERATION_MODE_ONEPASS;
//  global -> multi_host_mac_count = DOT1X_DEFAULT_MULTI_HOST_MAC_COUNT;
      global -> mac_is_authorized =FALSE;
    return global;
}

/*--------------------------------------------------
 * DOT1XMAC_VM_Do_Authenticator:
 * This function does an authenticator's job .. i.e. runs
 * the state machines .. one transition.
 *  Call this in a loop !
 *-------------------------------------------------- */
void DOT1XMAC_VM_Do_Authenticator( Global_Params * global ,UI32_T which)
{
     Auth_Pae          * auth_pae=NULL;
     BOOL_T     transitionResult;

     auth_pae = global -> theAuthenticator;
     global -> which_event = which ;

    if ( global == NULL )
    {
        lib1x_message(MESS_DBG_AUTHNET," Null argument received.");
        return ;
    }
//  if(global -> portEnabled == FALSE) /*kevin 2002/12/09*/
//      return;
    lib1x_authsm_listen_action( global ,which);     /* captures packets, updates variables.*/

    if(auth_pae -> state == apsm_Initialize)
    {
         transitionResult = DOT1XMAC_Trans_AuthSM( global );
         if ( transitionResult )
         {
             DOT1XMAC_Authsm_ExecuteAuthSM( global );
         }
    }
    if(auth_pae -> state == apsm_Reconnect)
    {
         transitionResult = DOT1XMAC_Trans_AuthSM( global );
         if ( transitionResult )
         {
                DOT1XMAC_Authsm_ExecuteAuthSM( global );
         }
    }

    transitionResult = DOT1XMAC_Trans_AuthSM( global );
    if ( transitionResult )
    {
         DOT1XMAC_Authsm_ExecuteAuthSM( global );
    }

    if(auth_pae -> state == apsm_Timeout )
    {
            transitionResult = DOT1XMAC_Trans_AuthSM( global );
            if ( transitionResult )
            {
                  DOT1XMAC_Authsm_ExecuteAuthSM( global );
            }
    }

    if(auth_pae -> state == apsm_Reconnect)
    {
            transitionResult = DOT1XMAC_Trans_AuthSM( global );
            if ( transitionResult )
            {
                  DOT1XMAC_Authsm_ExecuteAuthSM( global );
            }
    }

    if(auth_pae -> state == apsm_Connecting)
    {
            transitionResult = DOT1XMAC_Trans_AuthSM( global );
            if ( transitionResult )
            {
                  DOT1XMAC_Authsm_ExecuteAuthSM( global );
            }
    }

    if(auth_pae -> state == apsm_Disconnected)
    {
            transitionResult = DOT1XMAC_Trans_AuthSM( global );
            if ( transitionResult )
            {
                  DOT1XMAC_Authsm_ExecuteAuthSM( global );
            }
    }
    return;
}

/* These are transition functions for respective states of the Authenticator State Machine*/
static void DOT1XMAC_Authsm_Held( Auth_Pae * auth_pae , Global_Params * global)
{
    Bauth_SM * bauth_sm=NULL;
//  UI32_T quietperiod;

    bauth_sm = &(auth_pae -> bauth_sm);

    global -> portStatus = VAL_dot1xAuthAuthControlledPortStatus_unauthorized/*pst_Unauthorized*/;
    global -> currentId = bauth_sm -> idFromServer;
//printf("DOT1XMAC_Authsm_Held:id[%d]\n",global -> currentId);
    lib1x_auth_txCannedFail( auth_pae, global -> currentId, FALSE);
//  quietperiod = DOT1X_OM_Get_PortQuietPeriod(global->port_number);
//  auth_pae -> quietPeriod = quietperiod;
//  global -> timers -> quietWhile = quietperiod;//auth_pae -> quietPeriod;
//  auth_pae -> port_timers.quietWhile = quietperiod;//auth_pae -> quietPeriod;
    DOT1XMAC_Ptsm_SetTimer(global,DOT1XMAC_TIMER_TYPE_QUIETWHILE,1);
    auth_pae -> eapLogoff = FALSE;
    /*INC( global -> currentId );*/
    return;
}

static void DOT1XMAC_Authsm_Initialize( Auth_Pae * auth_pae , Global_Params * global)
{
      /*auth_pae -> state = apsm_Initialize;
    global -> currentId = 0;*/
    auth_pae-> portMode = VAL_dot1xAuthAuthControlledPortControl_auto;
    auth_pae -> reAuthCount = 0;
    global -> portStatus = VAL_dot1xAuthAuthControlledPortStatus_unauthorized/*pst_Unauthorized*/;
    global -> currentId = 1;
    //DOT1XMAC_Ptsm_SetTimer(global,DOT1XMAC_TIMER_TYPE_TXWHILE,1);
    auth_pae -> eapLogoff = FALSE;
    return;
}

static void DOT1XMAC_Authsm_Disconnected( Auth_Pae * auth_pae , Global_Params * global)
{
    /*global -> portStatus = VAL_dot1xAuthAuthControlledPortStatus_unauthorized;
    auth_pae -> eapLogoff = FALSE;
    auth_pae -> reAuthCount = 0;*/
    //SYSFUN_Sleep(3);
    lib1x_auth_txCannedFail( auth_pae , global -> currentId, FALSE );
    /*Need to do: free mac object*/
    //DOT1X_OM_Free_MacEntry_Working_Area(global -> theAuthenticator -> dot1x_packet_t.src_lport,global -> theAuthenticator -> dot1x_packet_t.src_mac,global -> theAuthenticator -> dot1x_packet_t.src_vid);
    return;
}

static void DOT1XMAC_Authsm_Connecting( Auth_Pae * auth_pae , Global_Params * global)
{
#if 0 /* JinhuaWei, 05 August, 2008 12:33:48 */
    UI32_T  txPeriod;
#endif /* #if 0 */

//  DOT1XMAC_OM_Get_TxPeriod(&txPeriod);
//  auth_pae -> txPeriod = txPeriod ;

    auth_pae-> eapStart = FALSE;
    global -> reAuthenticate = FALSE;
    //DOT1X_OM_Set_PortReAuthEnabled(global -> theAuthenticator -> dot1x_packet_t.src_lport ,FALSE);
    //global -> timers -> txWhen = auth_pae -> txPeriod;
    DOT1XMAC_Ptsm_SetTimer(global,DOT1XMAC_TIMER_TYPE_TXWHILE,1);
    auth_pae -> rxRespId = FALSE;
      //SYS_TIME_GetRealTimeBySec(&(global -> session_start_time));/*for MIB*/
    if(auth_pae -> reAuthCount < LIB1X_AP_REAUTHMAX)
      lib1x_auth_txReqId( auth_pae, global -> currentId );
    (auth_pae -> reAuthCount)++;
    return;
}

static void DOT1XMAC_Authsm_Reconnect( Auth_Pae * auth_pae, Global_Params * global)
{
    INC( global -> currentId );
//printf("DOT1XMAC_Authsm_Reconnect:currentId[%d]\n",global -> currentId);
    return;
}

static void DOT1XMAC_Authsm_Response( Auth_Pae * auth_pae, Global_Params * global)
{
    UI32_T server_timeout;
    Bauth_SM * bauth_sm=NULL;

    bauth_sm = &(auth_pae -> bauth_sm);
    server_timeout = DOT1X_OM_Get_AuthServerTimeout(global -> theAuthenticator -> dot1x_packet_t.src_lport);
    bauth_sm -> serverTimeout = server_timeout;

    global -> authTimeout = FALSE;
    //global -> timers -> aWhile = bauth_sm -> serverTimeout;
//      global -> timers -> aWhile = DOT1XMAC_DEFAULT_SERVER_TIMEOUT;
    DOT1XMAC_Ptsm_SetTimer(global,DOT1XMAC_TIMER_TYPE_SERVERWHILE,1);
    bauth_sm -> reqCount = 0;
    lib1x_bauthsm_sendRespToServer(global, global -> currentId);
    bauth_sm -> aReq = bauth_sm -> aSuccess = FALSE;
    bauth_sm -> rxResp = bauth_sm -> aFail = FALSE;
    return;
}

static void DOT1XMAC_Authsm_Request( Auth_Pae * auth_pae, Global_Params * global)
{
    UI32_T supplicant_timeout;
    Bauth_SM * bauth_sm=NULL;

    bauth_sm = &(auth_pae -> bauth_sm);
    supplicant_timeout = DOT1X_OM_Get_AuthSuppTimeout(global -> theAuthenticator -> dot1x_packet_t.src_lport);
//  bauth_sm -> suppTimeout = supplicant_timeout;

    global -> currentId = bauth_sm -> idFromServer;
//printf("DOT1XMAC_Authsm_Request:id[%d]\n",global -> currentId);
    lib1x_bauthsm_txReq(global, global -> currentId );
    //global -> timers -> aWhile = bauth_sm -> suppTimeout;
    DOT1XMAC_Ptsm_SetTimer(global,DOT1XMAC_TIMER_TYPE_SUPPLICANTWHILE,1);
    (bauth_sm -> reqCount)++;
    return;
}

static void DOT1XMAC_Authsm_Timeout( Auth_Pae * auth_pae, Global_Params * global)
{
    if ( global -> portStatus == VAL_dot1xAuthAuthControlledPortStatus_unauthorized/*pst_Unauthorized*/ )
    {
        lib1x_auth_txCannedFail( auth_pae, global -> currentId, FALSE );
    }
    global -> authTimeout = TRUE;
    return;
}

static void DOT1XMAC_Authsm_Authenticated( Auth_Pae * auth_pae , Global_Params * global)
{
    UI32_T  reAuthPeriod;
    Reauth_SM *reauth_sm=NULL;
    Bauth_SM * bauth_sm=NULL;

    bauth_sm = &(auth_pae -> bauth_sm);
    reauth_sm = &(auth_pae -> reauth_sm);
    DOT1X_OM_Get_ReAuthPeriod(&reAuthPeriod);
//  reauth_sm -> reAuthPeriod = reAuthPeriod;

    global -> portStatus = VAL_dot1xAuthAuthControlledPortStatus_authorized/*pst_Authorized*/;
    global -> currentId = bauth_sm -> idFromServer;
//printf("DOT1XMAC_Authsm_Authenticated:id[%d]\n",global -> currentId);
    lib1x_auth_txCannedSuccess( auth_pae, global -> currentId );
    //global -> timers -> reAuthWhen = reauth_sm -> reAuthPeriod;
    DOT1XMAC_Ptsm_SetTimer(global,DOT1XMAC_TIMER_TYPE_REAUTHWHILE,1);
    auth_pae -> reAuthCount = 0;

    return;
}

static void DOT1XMAC_Authsm_Close( Auth_Pae * auth_pae , Global_Params * global)
{
      /*free MAC entry*/
      if(DOT1X_OM_Free_MacEntry_Working_Area(auth_pae -> dot1x_packet_t.src_lport,auth_pae -> dot1x_packet_t.src_mac,auth_pae -> dot1x_packet_t.src_vid)==FALSE)
      {
            lib1x_message( MESS_DBG_SPECIAL,"DOT1XMAC_Authsm_Close: Free MAC entry Error!\n");
      }
    return;
}

static void DOT1XMAC_Authsm_ForceUnauth( Auth_Pae * auth_pae , Global_Params * global)
{
    auth_pae -> AuthSessionStatsEntry.dot1xAuthSessionTerminateCause = VAL_dot1xAuthSessionTerminateCause_authControlForceUnauth;
    global -> portStatus = VAL_dot1xAuthAuthControlledPortStatus_unauthorized/*pst_Unauthorized*/;
    auth_pae-> portMode = VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized;
    auth_pae -> eapStart = FALSE;
    //SYSFUN_Sleep(3);
    lib1x_auth_txCannedFail( auth_pae, global -> currentId, FALSE );
//printf("DOT1XMAC_Authsm_ForceUnauth:id++\n");
    INC( global -> currentId );
    return;
}

static void DOT1XMAC_Authsm_ForceAuth( Auth_Pae * auth_pae , Global_Params * global)
{
//#if 0
    //DOT1XMAC_PortEntry_T *port_entry_ptr;

    if( auth_pae -> eapLogoff == TRUE)
    {
            /* remove MAC from L2 MAC table */
            //if(global -> mac_is_authorized == TRUE)
            {
                 if(DOT1X_OM_Delete_Authenticated_MAC(global -> theAuthenticator -> dot1x_packet_t.src_lport,global -> theAuthenticator -> dot1x_packet_t.src_mac,global->authorized_vid) == TRUE)
                 {
                     lib1x_remove_portmac_from_psec(global -> authorized_vid, global -> theAuthenticator -> dot1x_packet_t.src_mac);
                 }
                  //DOT1XMAC_RemovePortMacFromPSEC(global -> src_lport,global -> src_mac,global ->src_vid);
                  global -> mac_is_authorized = FALSE;
                  //port_entry_ptr = DOT1XMAC_OM_Get_PortEntry(global -> src_lport);
                  //(port_entry_ptr -> authorized_mac_counter)--;
            }
            auth_pae -> eapLogoff = FALSE;
    }
    else
    {
            global -> portStatus = VAL_dot1xAuthAuthControlledPortStatus_authorized/*pst_Authorized*/;
            auth_pae -> portMode = VAL_dot1xAuthAuthControlledPortControl_forceAuthorized;
            auth_pae -> eapStart = FALSE;
            //SYSFUN_Sleep(3);
            lib1x_auth_txCannedSuccess( auth_pae, global -> currentId );
            INC( global -> currentId );
//printf("DOT1XMAC_Authsm_ForceAuth:id++[%d]\n",global -> currentId);
    }
//#endif
    return;
}

/*--------------------------------------------------
 * DOT1XMAC_Authsm_ExecuteAuthSM : This function executes the code on entry to a state.
 *--------------------------------------------------*/
static void DOT1XMAC_Authsm_ExecuteAuthSM( Global_Params * global )
{
      Auth_Pae * auth_pae = global -> theAuthenticator;

    switch( auth_pae -> state )
    {
        case  apsm_Held:
                    lib1x_message(MESS_DBG_AUTHNET, "MAC-based:Into Held");
                              DOT1XMAC_Authsm_Held( auth_pae , global);
                              break;
        case    apsm_Initialize:
                    lib1x_message(MESS_DBG_AUTHNET, "MAC-based:Into Initialize");
                    DOT1XMAC_Authsm_Initialize( auth_pae , global);
                    break;
        case    apsm_Disconnected:
                    lib1x_message(MESS_DBG_AUTHNET, "MAC-based:Into Disconnected");
                    DOT1XMAC_Authsm_Disconnected( auth_pae, global );
                    break;
        case    apsm_Connecting:
                    lib1x_message(MESS_DBG_AUTHNET, "MAC-based:Into Connecting");
                    (auth_pae -> AuthDiagEntry.dot1xAuthEntersConnecting)++;/*for MIB*/
                    DOT1XMAC_Authsm_Connecting( auth_pae , global);
                    break;
        case  apsm_Response:
                    lib1x_message(MESS_DBG_AUTHNET, "MAC-based:Into Response");
                              DOT1XMAC_Authsm_Response( auth_pae , global);
                              break;
        case  apsm_Request:
                    lib1x_message(MESS_DBG_AUTHNET, "MAC-based:Into Request");
                              DOT1XMAC_Authsm_Request( auth_pae , global);
                              break;
        case  apsm_Timeout:
                    lib1x_message(MESS_DBG_AUTHNET, "MAC-based:Into Timeout");
                              DOT1XMAC_Authsm_Timeout( auth_pae , global);
                               break;
        case  apsm_Reconnect:
                    lib1x_message(MESS_DBG_AUTHNET, "MAC-based:Into Reconnect");
                              DOT1XMAC_Authsm_Reconnect( auth_pae , global);
                              break;
        case    apsm_Authenticated:
                    lib1x_message(MESS_DBG_AUTHNET, "MAC-based:Into Authenticated");
                    DOT1XMAC_Authsm_Authenticated( auth_pae, global);
                    break;
            case    apsm_Close:
                    lib1x_message(MESS_DBG_AUTHNET, "MAC-based:Into Close");
                    DOT1XMAC_Authsm_Close( auth_pae, global);
                    break;
        case    apsm_Force_Auth:
                    lib1x_message(MESS_DBG_AUTHNET, "MAC-based:Into Force Auth");
                    DOT1XMAC_Authsm_ForceAuth( auth_pae, global);
                    break;
        case    apsm_Force_Unauth:
                    lib1x_message(MESS_DBG_AUTHNET, "MAC-based:Into Force Unauth");
                    DOT1XMAC_Authsm_ForceUnauth( auth_pae, global);
                    break;
            default:
                              break;
    }
     return;
}


/*--------------------------------------------------
 * DOT1XMAC_Trans_AuthSM :
 * This function transitions the auth pae state
 * machine.
 *-------------------------------------------------- */

static BOOL_T DOT1XMAC_Trans_AuthSM( Global_Params * global )
{
      BOOL_T      transitionDone = FALSE;
    BOOL_T      reAuthEnabled;
    Auth_Pae    * auth_pae=NULL;
    Bauth_SM    *bauth_sm=NULL;
//  UI32_T          current_time;
//  DOT1XMAC_PortEntry_T    *port_entry_ptr;
    UI8_T       flag;
    UI32_T      system_auth_control;
    BOOL_T      timeout_status = FALSE;
    UI32_T      maxReq;
    UI32_T      portControl;

    auth_pae = global -> theAuthenticator;
    bauth_sm = &(auth_pae -> bauth_sm);
    if ( ! auth_pae -> isSuppPresent ) return FALSE;
    /* Check Global Conditions Here.*/

//  port_entry_ptr = DOT1XMAC_OM_Get_PortEntry(global -> src_lport);
    portControl = DOT1X_OM_Get_PortControlMode(global -> theAuthenticator -> dot1x_packet_t.src_lport);//port_entry_ptr -> mode ;
    system_auth_control = DOT1X_OM_Get_SystemAuthControl();
    if (system_auth_control == VAL_dot1xPaeSystemAuthControl_enabled)
            global -> portEnabled = TRUE;
    else
        global -> portEnabled = FALSE;
    //global -> portEnabled = system_enable_status;/*port_entry_ptr -> enable_status;*/

    /* Condition 0:*/
    /* if nedd to check eaplogoff here ? */
//  if(auth_pae -> eapLogoff)
//  {
//      auth_pae -> state = apsm_Disconnected;
//      return TRUE;
//  }
    /* Condition 1:*/
    if ((( portControl == VAL_dot1xAuthAuthControlledPortControl_auto ) && ( auth_pae -> portMode != portControl )) || (global -> initialize ) || !( global -> portEnabled) )
    {
        auth_pae -> state = apsm_Initialize;
        global -> initialize = FALSE;
        return TRUE;
    }

    /* Condition 2:Force Authorized*/
    if ( ( portControl == VAL_dot1xAuthAuthControlledPortControl_forceAuthorized ) &&
         ( auth_pae -> portMode != portControl ) &&
         ( ! global -> initialize ) && ( global -> portEnabled ) )
    {
        auth_pae -> state = apsm_Force_Auth;
        return TRUE;
    }

    /* Condition 3:Force Unauthorized*/
    if ( ( portControl == VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized ) &&
         ( auth_pae -> portMode != portControl ) &&
         ( ! global -> initialize ) && ( global -> portEnabled ) )
    {
        auth_pae -> state = apsm_Force_Unauth;
        return TRUE;
    }

      switch( auth_pae -> state )
    {

            case    apsm_Initialize:
                        auth_pae -> state = apsm_Connecting;/* Unconditional transfer */
                        transitionDone = TRUE;
                        break;
        case    apsm_Disconnected:
                        auth_pae -> state = apsm_Close;
                        transitionDone = TRUE;
                        break;
        case    apsm_Connecting:
                        if ( auth_pae -> eapLogoff )/*eapLogoff*/
                        {
                              auth_pae -> state = apsm_Disconnected;
                              transitionDone = TRUE;
                              break;
                        }

                        /* reAuthCount > reAuthMax */
                        if ( auth_pae -> reAuthCount > auth_pae -> reAuthMax )
                        {
                              auth_pae -> state = apsm_Disconnected;
                              transitionDone = TRUE;
                              break;
                        }
                        /* rxRespId */
                        if ( auth_pae -> rxRespId  && ( auth_pae -> reAuthCount <= auth_pae -> reAuthMax ) )
                        {
                              auth_pae -> state = apsm_Response;
                              transitionDone = TRUE;
                              break;
                        }
                        /* timeout || eapStart || reAuthenticate*/
                        if(global -> which_event == DOT1X_PTSM_TIMEOUT)
                        {
                              if(DOT1XMAC_Ptsm_IsTimeout(global -> theAuthenticator -> dot1x_packet_t.src_lport,DOT1XMAC_TIMER_TYPE_TXWHILE))
                              {
                                    DOT1XMAC_Ptsm_GetTimer(global,DOT1XMAC_TIMER_TYPE_TXWHILE,&flag);
                                    if(flag == 0)
                                          DOT1XMAC_Ptsm_SetTimer(global,DOT1XMAC_TIMER_TYPE_TXWHILE,1);
                                    else
                                          timeout_status = TRUE;/*DOT1XMAC_Ptsm_IsTimeout(DOT1XMAC_TIMER_TYPE_TXWHILE); */
                              }
                              global -> which_event = DOT1X_PTSM_NONE;
                        }
                        if ( ( (timeout_status == TRUE)/*( global -> timers -> txWhen == 0 )*/ || auth_pae -> eapStart || global -> reAuthenticate) && ( auth_pae -> reAuthCount <= auth_pae -> reAuthMax ) )
                        {
                              auth_pae -> state = apsm_Connecting;
                              transitionDone = TRUE;
                              DOT1XMAC_Ptsm_SetTimer(global,DOT1XMAC_TIMER_TYPE_TXWHILE,0);
                              break;
                        }
                        break;

        case    apsm_Response:
                        if ( auth_pae -> eapLogoff )/*eapLogoff*/
                        {
                              auth_pae -> state = apsm_Disconnected;
                              transitionDone = TRUE;
                              break;
                        }
                        if ( auth_pae -> eapStart || global -> reAuthenticate)/*eapStart || reAuthenticate*/
                        {
                              auth_pae -> state = apsm_Reconnect;
                              transitionDone = TRUE;
                              break;
                        }
                        if ( bauth_sm -> aReq ) /*rxReq*/
                        {
                              auth_pae -> state = apsm_Request;
                              transitionDone = TRUE;
                              break;
                        }
                #if 0   /*server timeout = RADIUS Request Timeout * Retransmit Times*/
                    if(global -> which_event == DOT1X_PTSM_TIMEOUT)
                        {
                            if(DOT1XMAC_Ptsm_IsTimeout(global -> theAuthenticator -> dot1x_packet_t.src_lport,DOT1XMAC_TIMER_TYPE_SERVERWHILE))
                                {
                                    DOT1XMAC_Ptsm_GetTimer(global,DOT1XMAC_TIMER_TYPE_SERVERWHILE,&flag);
                                    if(flag == 0)
                                        DOT1XMAC_Ptsm_SetTimer(global,DOT1XMAC_TIMER_TYPE_SERVERWHILE,1);
                                    else
                                        timeout_status = TRUE;/*DOT1XMAC_Ptsm_IsTimeout(DOT1XMAC_TIMER_TYPE_SERVERWHILE);   */

                                    //printf(" \nServer Timeout flag=%d \n",flag);
                                }
                            global -> which_event = DOT1X_PTSM_NONE;
                        }
                #endif
#if 1
                if ( global-> theAuthenticator-> radius_packet_t.radius_auth_result == 1/*TIMEOUT_RC*/)/*timeout*/
                {
                     auth_pae -> state = apsm_Timeout;
                     transitionDone = TRUE;
                     DOT1XMAC_Ptsm_SetTimer(global,DOT1XMAC_TIMER_TYPE_SERVERWHILE,0);
                     break;
                }
#endif
                if ( bauth_sm -> aFail )/*rxaFail*/
                {
                     auth_pae -> state = apsm_Held;
                     transitionDone = TRUE;
                     break;
                }
                if ( bauth_sm -> aSuccess )/*rxaSuccess*/
                {
                     auth_pae -> state = apsm_Authenticated;
                     transitionDone = TRUE;
                     break;
                }
                break;
        case    apsm_Request:
                if ( auth_pae -> eapLogoff )/*eapLogoff*/
                {
                    auth_pae -> state = apsm_Disconnected;
                    transitionDone = TRUE;
                    break;
                }
                if ( bauth_sm -> rxResp ) /*rxResp*/
                {
                     auth_pae -> state = apsm_Response;
                     transitionDone = TRUE;
                     break;
                }

                if(global -> which_event == DOT1X_PTSM_TIMEOUT)
                    {
                        if(DOT1XMAC_Ptsm_IsTimeout(global -> theAuthenticator -> dot1x_packet_t.src_lport,DOT1XMAC_TIMER_TYPE_SUPPLICANTWHILE))
                        {
                            DOT1XMAC_Ptsm_GetTimer(global,DOT1XMAC_TIMER_TYPE_SUPPLICANTWHILE,&flag);
                            if(flag == 0)
                                DOT1XMAC_Ptsm_SetTimer(global,DOT1XMAC_TIMER_TYPE_SUPPLICANTWHILE,1);
                            else
                                timeout_status = TRUE;/*DOT1XMAC_Ptsm_IsTimeout(DOT1XMAC_TIMER_TYPE_SUPPLICANTWHILE);   */
                        }
                        global -> which_event = DOT1X_PTSM_NONE;
                    }

                maxReq = DOT1X_OM_Get_PortMaxReq(global -> theAuthenticator -> dot1x_packet_t.src_lport);
                //bauth_sm -> maxReq = max_req;
                if ( ( timeout_status == TRUE) && ( bauth_sm -> reqCount < maxReq ) )/*timeout*/
                {
                     /* No change in state*/
                     transitionDone = TRUE;
                     DOT1XMAC_Ptsm_SetTimer(global,DOT1XMAC_TIMER_TYPE_SUPPLICANTWHILE,0);
                     break;
                }
                if ( ( timeout_status == TRUE) && ( bauth_sm -> reqCount >= maxReq ) )/*reqCount > maxReq */
                {
                     auth_pae -> state = apsm_Timeout;
                     transitionDone = TRUE;
                     DOT1XMAC_Ptsm_SetTimer(global,DOT1XMAC_TIMER_TYPE_SUPPLICANTWHILE,0);
                     break;
                }
                break;

        case    apsm_Authenticated:
                if(global -> which_event == DOT1X_PTSM_TIMEOUT)
                    {
                        timeout_status = DOT1XMAC_Ptsm_IsTimeout(global -> theAuthenticator -> dot1x_packet_t.src_lport,DOT1XMAC_TIMER_TYPE_REAUTHWHILE);
                        if(timeout_status == TRUE)
                        {
                            //DOT1X_OM_Get_ReAuthenticationMode(&reAuthEnabled);
                            reAuthEnabled = DOT1X_OM_Get_PortReAuthEnabled(global -> theAuthenticator -> dot1x_packet_t.src_lport);
                            DOT1XMAC_Ptsm_GetTimer(global,DOT1XMAC_TIMER_TYPE_REAUTHWHILE,&flag);
                            if(flag == 0)
                                DOT1XMAC_Ptsm_SetTimer(global,DOT1XMAC_TIMER_TYPE_REAUTHWHILE,1);
                            else
                                {
                                //  timeout_status = DOT1XMAC_Ptsm_IsTimeout(DOT1XMAC_TIMER_TYPE_REAUTHWHILE);
                                    if(/*(timeout_status == TRUE) &&*/ reAuthEnabled)
                                    {
                                        global -> reAuthenticate = TRUE;
                                        DOT1XMAC_Ptsm_SetTimer(global,DOT1XMAC_TIMER_TYPE_REAUTHWHILE,0);
                                    }
                                }
                            global -> which_event = DOT1X_PTSM_NONE;
                        }
                    }

                if ( auth_pae -> eapStart || global -> reAuthenticate  )/*eapStart or timeout*/
                {
                    auth_pae -> state = apsm_Reconnect;
                    transitionDone = TRUE;
                    break;
                }

                if ( auth_pae -> eapLogoff )/*eapLogoff*/
                {
                    auth_pae -> state = apsm_Disconnected;
                    transitionDone = TRUE;
                    break;
                }
                break;

        case    apsm_Held:
                if ( auth_pae -> eapLogoff )/*eapLogoff*/
                {
                    auth_pae -> state = apsm_Disconnected;
                    transitionDone = TRUE;
                    break;
                }
                if(global -> which_event == DOT1X_PTSM_TIMEOUT)
                    {
                        if(DOT1XMAC_Ptsm_IsTimeout(global -> theAuthenticator -> dot1x_packet_t.src_lport,DOT1XMAC_TIMER_TYPE_QUIETWHILE))
                                    {
                            DOT1XMAC_Ptsm_GetTimer(global,DOT1XMAC_TIMER_TYPE_QUIETWHILE,&flag);
                            if(flag == 0)
                                DOT1XMAC_Ptsm_SetTimer(global,DOT1XMAC_TIMER_TYPE_QUIETWHILE,1);
                            else
                                timeout_status = TRUE;/*DOT1XMAC_Ptsm_IsTimeout(DOT1XMAC_TIMER_TYPE_QUIETWHILE);    */
                        }
                        global -> which_event = DOT1X_PTSM_NONE;
                    }
                if ( timeout_status == TRUE)/*timeout*/
                {
                    auth_pae -> state = apsm_Reconnect/*apsm_Timeout*/;
                    transitionDone = TRUE;
                    DOT1XMAC_Ptsm_SetTimer(global,DOT1XMAC_TIMER_TYPE_QUIETWHILE,0);
                }
                break;

        case    apsm_Timeout:
                auth_pae -> state = apsm_Reconnect; /* Unconditional transfer */
                transitionDone = TRUE;
                break;

        case    apsm_Reconnect:
                auth_pae -> state = apsm_Connecting; /* Unconditional transfer */
                transitionDone = TRUE;
                break;

        case    apsm_Close:
                auth_pae -> state = apsm_Close;
                transitionDone = TRUE;/*no other state can do*/
                break;
        case    apsm_Force_Unauth:
                if ( auth_pae -> eapStart )
                    transitionDone = TRUE;          /* New state is also the same */
                break;

        case    apsm_Force_Auth:
                if ( auth_pae -> eapStart )
                    transitionDone = TRUE;          /* New state is also the same */
                if ( auth_pae -> eapLogoff )
                    transitionDone = TRUE;
                break;
        default:
                    break;
    }
    return transitionDone;
}

