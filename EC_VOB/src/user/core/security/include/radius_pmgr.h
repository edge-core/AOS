/*---------------------------------------------------------------------------
 * Module   : radius_pmgr.h
 *---------------------------------------------------------------------------
 * PURPOSE  : Provide APIs to access RADIUS.
 *---------------------------------------------------------------------------
 * NOTES    :
 *
 *---------------------------------------------------------------------------
 * HISTORY  : 08/15/2007 - Wakka Tu, Create
 *
 *---------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007
 *---------------------------------------------------------------------------
 */

#ifndef RADIUS_PMGR_H
#define RADIUS_PMGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "radius_type.h"
#include "radius_mgr.h"

/* NAME CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*---------------------------------------------------------------------------
 * FUNCTION NAME - RADIUS_PMGR_InitiateProcessResources
 *---------------------------------------------------------------------------
 * PURPOSE : Initiate resource used in the calling process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_InitiateProcessResources(void);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_PMGR_Set_Request_Timeout
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the number of times the RADIUS client
 *           waits for a reply from RADIUS server
 * INPUT:    timeout value.
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_Set_Request_Timeout( UI32_T timeval );

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_PMGR_Set_Server_Port
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the UDP port number of the remote RADIUS server
 * INPUT:    Prot number
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_Set_Server_Port(UI32_T serverport);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_PMGR_Set_Server_Secret
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the shared secret text string used between
 *           the RADIUS client and server.
 * INPUT:    secret text string
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_Set_Server_Secret( UI8_T * serversecret );

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_PMGR_Set_Retransmit_Times
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the number of times the RADIUS client
 *           transmits each RADIUS request to the server before giving up
 * INPUT:    retransmit times
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_Set_Retransmit_Times(UI32_T retryval);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_PMGR_Set_Server_IP
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the IP address of the remote RADIUS server
 * INPUT:    RADIUS server IP address
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_Set_Server_IP(UI32_T serverip);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_PMGR_Auth_Check
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client to do authentication
 * INPUT:    RADIUS Authentication username  and password
 * OUTPUT:   RADIUS Authentication privilege
 *           privilege: RADIUS client service type
 *            15 = AUTH_ADMINISTRATIVE
 *             0 = AUTH_LOGIN
 * RETURN:   RADIUS Authentication result
 *        -3 = not in master mode
 *            -2 = receive illegal packet
 *            -1 = Authentication failure
 *             0 = Authentication OK
 *             1 = Authentication timeout
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
I32_T RADIUS_PMGR_Auth_Check(UI8_T *username,UI8_T *password,I32_T *privilege, UI32_T cookie); /*maggie liu for RADIUS authentication ansync*/


/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_PMGR_AsyncLoginAuth
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client to do authentication
 * INPUT:    username       - user name
 *           password       - password
 *           cookie         - caller cookie
 *           cookie_size    - size of cookie
 * OUTPUT:   None
 * RETURN:   TRUE - request is send successfully; FALSE - IPC error
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_AsyncLoginAuth(
    const char *username,
    const char *password,
    void *cookie,
    UI32_T cookie_size
);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_PMGR_AsyncEnablePasswordAuth
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client to do enable authentication
 * INPUT:    username       - user name
 *           password       - password
 *           cookie         - caller cookie
 *           cookie_size    - size of cookie
 * OUTPUT:   None
 * RETURN:   TRUE - request is send successfully; FALSE - IPC error
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_AsyncEnablePasswordAuth(
    const char *username,
    const char *password,
    void *cookie,
    UI32_T cookie_size);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_PMGR_AsyncEapAuthCheck
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client to do EAP authentication
 * INPUT:    eap_data     --- EAP packet data
 *           eap_datalen  --- EAP packet data length
 *           radius_id    --- RADIUS sequent ID
 *           state_data   --- RADIUS STATE type packet data
 *           state_datale --- RADIUS STATE type packet data length
 *           src_port     --- source port
 *           src_mac      --- source mac address
 *           src_vid      --- source vid
 *           cookie       --- MSGQ_ID for return result
 *           service_type --- which component need to be service
 *           server_ip    --- Use this server IP address first. 0 means not
 *                            specified
 *           username_p   --- User name
 *           flag         --- Control flag
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_AsyncEapAuthCheck(
    UI8_T   *eap_data,
    UI32_T  eap_datalen,
    UI32_T  radius_id,
    UI8_T   *state_data,
    UI32_T  state_datalen,
    UI32_T  src_port,
    UI8_T   *src_mac,
    UI32_T  src_vid,
    UI32_T  cookie,
    UI32_T  service_type,
    UI32_T  server_ip,
    char    *username_p,
    RADIUS_AsyncRequestControlFlag_T flag);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_PMGR_Set_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the RADIUS server host
 * INPUT:    server_index,server_host
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:        server_ip;
 *      server_port (1-65535)  - set 0 will use the global radius configuration
 *          timeout     (1-65535)  - set 0 will use the global radius configuration
 *          retransmit  (1-65535)  - set 0 will use the global radius configuration
 *          secret      (length < RADIUS_MAX_SECRET_LENGTH)  - set NULL will use the global radius configuration
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_Set_Server_Host(UI32_T server_index,RADIUS_Server_Host_T *server_host);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_PMGR_Destroy_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will destroy the RADIUS server host
 * INPUT:    server_index
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_Destroy_Server_Host(UI32_T server_index);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_PMGR_SetServerAcctPort
 *---------------------------------------------------------------------------
 * PURPOSE  : set global server accounting port
 * INPUT    : acct_port
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_SetServerAcctPort(UI32_T acct_port);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_PMGR_RadaAuthCheck
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client to do RADA authentication
 * INPUT:    src_port, src_mac, rada_username, rada_passwd,
 *           cookie (MSGQ_ID to return the result)
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_RadaAuthCheck(
    UI32_T  src_port,       UI8_T   *src_mac,
    char    *rada_username, char    *rada_passwd,   UI32_T cookie);

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_PMGR_GetAccClientIdentifier
 *---------------------------------------------------------------------------
 * PURPOSE  : This function to get the NAS-Identifier of the RADIUS accounting client.
 * INPUT    : none.
 * OUTPUT   : client_identifier  --  the NAS-Identifier of the RADIUS accounting client.
 * RETURN   : TRUE to indicate successful and FALSE to indicate failure.
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_GetAccClientIdentifier(RADACC_AccClientIdentifier_T *client_identifier);
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_PMGR_SyncAuthCheck
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client to do authentication
 * INPUT:    RADIUS Authentication username  and password
 * OUTPUT:   RADIUS Authentication privilege
 *           privilege: RADIUS client service type
 *            15 = AUTH_ADMINISTRATIVE
 *             0 = AUTH_LOGIN
 * RETURN:   RADIUS Authentication result
 *            -3 = not in master mode
 *            -2 = receive illegal packet
 *            -1 = Authentication failure
 *             0 = Authentication OK
 *             1 = Authentication timeout
 * NOTE:     The same as RADIUS_Auth_Check
 *---------------------------------------------------------------------------
 */
I32_T RADIUS_PMGR_SyncAuthCheck(UI8_T *username,UI8_T *password,I32_T *privilege);

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_PMGR_SubmitRequest
 *---------------------------------------------------------------------------
 * PURPOSE  : Submit a request, call RADIUS_MGR_SubmitRequest to create a
 *            request in request queue.
 * INPUT    : request_p
 * OUTPUT   : None
 * RETURN   : RADIUS_RETURN_FAIL
 *            RADIUS_RETURN_SUCCESS
 * NOTES    :
 *---------------------------------------------------------------------------
 */
BOOL_T
RADIUS_PMGR_SubmitRequest(
    RADIUS_MGR_RequestContext_T request_p
    );


#endif /* RADIUS_PMGR_H */

