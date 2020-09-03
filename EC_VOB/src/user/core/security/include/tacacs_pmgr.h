#ifndef TACACS_PMGR_H
#define TACACS_PMGR_H
#include "sys_type.h"
#include "tacacs_type.h"
#include "tacacs_mgr.h"

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TACACS_PMGR_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for TACACS_PMGR.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void TACACS_PMGR_Init(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_PMGR_Set_Server_Port
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the TCP port number of the remote TACACS server
 * INPUT:    Prot number
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_PMGR_Set_Server_Port(UI32_T serverport);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_PMGR_SetServerRetransmit
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the global retransmit times of the remote
 *           TACACS server.
 * INPUT:    retransmit times.
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_PMGR_SetServerRetransmit(UI32_T retransmit);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_PMGR_SetServerTimeout
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the global timeout of the remote
 *           TACACS server.
 * INPUT:    timeout.
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_PMGR_SetServerTimeout(UI32_T timeout);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_PMGR_Set_Server_Secret
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the shared secret text string used between
 *           the TACACS client and server.
 * INPUT:    secret text string
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_PMGR_Set_Server_Secret(UI8_T *serversecret);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_PMGR_Set_Server_IP
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the IP address of the remote TACACS server
 * INPUT:    TACACS server IP address
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_PMGR_Set_Server_IP(UI32_T serverip);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_PMGR_AnsyncLoginAuth
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call TACACS client to do authentication
 * INPUT:    username       - user name
 *           password       - password
 *           sess_type      - session type
 *           sess_id        - session ID
 *           rem_ip_addr    - remote IP address
 *           cookie         - caller cookie
 *           cookie_size    - size of cookie
 * OUTPUT:   None
 * RETURN:   TRUE - request is send successfully; FALSE - IPC error
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_PMGR_AnsyncLoginAuth(
    const char *username,
    const char *password,
    UI32_T sess_type,
    UI32_T sess_id,
    const L_INET_AddrIp_T *rem_ip_addr,
    void *cookie,
    UI32_T cookie_size);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_PMGR_AnsyncEnablePasswordAuth
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call TACACS client to do enable authentication
 * INPUT:    username       - user name
 *           password       - password
 *           sess_type      - session type
 *           sess_id        - session ID
 *           rem_ip_addr    - remote IP address
 *           cookie         - caller cookie
 *           cookie_size    - size of cookie
 * OUTPUT:   None
 * RETURN:   TRUE - request is send successfully; FALSE - IPC error
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_PMGR_AnsyncEnablePasswordAuth(
    const char *username,
    const char *password,
    UI32_T sess_type,
    UI32_T sess_id,
    L_INET_AddrIp_T *rem_ip_addr,
    void *cookie,
    UI32_T cookie_size);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_PMGR_Set_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the TACACS server host
 * INPUT:    server_index (1-based), server_host
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:    server_ip;
 *		    server_port (1-65535)  - set 0 will use the global TACACS configuration
 *       	timeout     (1-65535)  - set 0 will use the global TACACS configuration
 *       	retransmit  (1-65535)  - set 0 will use the global TACACS configuration
 *        	secret      (length < TACACS_MAX_SECRET_LENGTH)  - set NULL will use the global TACACS configuration
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_PMGR_Set_Server_Host(UI32_T server_index,TACACS_Server_Host_T *server_host);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_PMGR_SetServerHostByIpAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : setup server host by server_ip
 * INPUT    : server_ip, server_port, timeout, retransmit, secret, acct_port
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if server_port, timeout, retransmit, acct_port == 0 then use global value
 *            if strlen(secret) == 0 then use global value
 *            if specified ip doesn't exist, then create it. or modify it
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_PMGR_SetServerHostByIpAddress(TACACS_Server_Host_T *server_host);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_PMGR_Destroy_Server_Host_By_Index
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will destroy the TACACS server host
 * INPUT:    server_index (1-based)
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_PMGR_Destroy_Server_Host_By_Index(UI32_T server_index);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_PMGR_Destroy_Server_Host_By_Ip_Address
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will destroy the TACACS server host
 * INPUT:    server_ip
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_PMGR_Destroy_Server_Host_By_Ip_Address(UI32_T server_ip);

#if (SYS_CPNT_AUTHORIZATION == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_PMGR_Author_Check
 *-------------------------------------------------------------------------
 * PURPOSE  : do authorization function
 * INPUT    : *request -- input of authorization request
 * OUTPUT   : *reply -- output of authorization request
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------
 */
BOOL_T TACACS_PMGR_Author_Check(TACACS_AuthorRequest_T *request,TACACS_AuthorReply_T *reply);
#endif

#endif /*#ifndef TACACS_PMGR_H*/

